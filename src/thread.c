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

static int	join_threads(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}

static t_error	stop_before_start(t_sim *sim, int created)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->stop = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->state_lock);
	join_threads(sim, created);
	return (ERR_THREAD);
}

t_error	run_threads(t_sim *sim)
{
	int	i;
	int	failed;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (stop_before_start(sim, i));
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (stop_before_start(sim, i));
	start_simulation(sim);
	failed = join_threads(sim, i);
	if (pthread_join(sim->monitor, NULL) != 0)
		failed = 1;
	if (failed)
		return (ERR_THREAD);
	return (SUCCESS);
}
