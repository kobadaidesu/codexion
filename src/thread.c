#include "../includes/codexion.h"

static int	wait_start(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	while (!sim->started && !sim->stop)
		pthread_cond_wait(&sim->start_cond, &sim->state_lock);
	if (sim->stop)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}
	pthread_mutex_unlock(&sim->state_lock);
	return (1);
}

static void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (!wait_start(coder))
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

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (stop_before_start(sim, i));
		i++;
	}
	pthread_mutex_lock(&sim->state_lock);
	sim->start_time = get_time_us();
	sim->started = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->state_lock);
	if (join_threads(sim, i))
		return (ERR_THREAD);
	return (SUCCESS);
}
