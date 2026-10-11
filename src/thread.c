#include "../includes/codexion.h"

/*
** thread.c : thread生成と一斉スタートゲート、join
** create失敗時はstopを立てて起こしてから作成済み分をjoinする。
*/

int	wait_start(t_sim *sim)
{
	int	ready;

	pthread_mutex_lock(&sim->state_lock);
	while (!sim->started && !sim->stop)
		pthread_cond_wait(&sim->start_cond, &sim->state_lock);
	ready = !sim->stop;
	pthread_mutex_unlock(&sim->state_lock);
	return (ready);
}

static void	join_threads(t_coder *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
}

static t_error	stop_before_start(t_sim *sim, t_coder *coders, int created)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->stop = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->state_lock);
	join_threads(coders, created);
	return (ERR_FATAL);
}

static t_error	create_coders(t_sim *sim, t_coder *coders)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&coders[i].thread, NULL,
				coder_routine, &coders[i]) != 0)
			return (stop_before_start(sim, coders, i));
		i++;
	}
	return (SUCCESS);
}

t_error	run_threads(t_sim *sim, t_coder *coders)
{
	pthread_t	monitor;
	int			i;

	if (create_coders(sim, coders) != SUCCESS)
		return (ERR_FATAL);
	if (pthread_create(&monitor, NULL, monitor_routine, coders) != 0)
		return (stop_before_start(sim, coders,
				sim->config.number_of_coders));
	pthread_mutex_lock(&sim->state_lock);
	sim->start_time = get_time_us();
	i = sim->config.number_of_coders;
	while (i > 0)
	{
		i--;
		coders[i].last_compile_start = sim->start_time;
	}
	sim->started = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->state_lock);
	join_threads(coders, sim->config.number_of_coders);
	pthread_join(monitor, NULL);
	return (SUCCESS);
}
