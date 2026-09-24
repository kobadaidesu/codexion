#include "../includes/codexion.h"

/*
** thread.c : thread生成と一斉スタートゲート、join
** 全threadはwait_startで待ち、start_simulationのbroadcastで同時に開始。
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

static void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (!wait_start(coder->sim))
		return (NULL);
	while (coder->compiles
		< coder->sim->config.number_of_compiles_required)
	{
		if (!coder_cycle(coder))
			break ;
	}
	return (NULL);
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

t_error	run_threads(t_sim *sim, t_coder *coders)
{
	pthread_t	monitor;
	int			i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&coders[i].thread, NULL,
				coder_routine, &coders[i]) != 0)
			return (stop_before_start(sim, coders, i));
		i++;
	}
	if (pthread_create(&monitor, NULL, monitor_routine, coders) != 0)
		return (stop_before_start(sim, coders, i));
	start_simulation(sim, coders);
	join_threads(coders, i);
	pthread_join(monitor, NULL);
	return (SUCCESS);
}
