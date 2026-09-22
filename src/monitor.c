#include "../includes/codexion.h"

static void	wait_monitor_start(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_lock);
	while (!sim->started && !sim->stop)
		pthread_cond_wait(&sim->start_cond, &sim->state_lock);
	pthread_mutex_unlock(&sim->state_lock);
}

static int	find_burned_coder(t_sim *sim, long long now)
{
	long long	deadline;
	int			i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (sim->coders[i].compiles
			< sim->config.number_of_compiles_required)
		{
			deadline = sim->coders[i].last_compile_start
				+ sim->config.time_to_burnout * 1000;
			if (now >= deadline)
				return (i);
		}
		i++;
	}
	return (-1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		burned;

	sim = (t_sim *)arg;
	wait_monitor_start(sim);
	while (1)
	{
		pthread_mutex_lock(&sim->state_lock);
		if (sim->stop)
		{
			pthread_mutex_unlock(&sim->state_lock);
			break ;
		}
		burned = find_burned_coder(sim, get_time_us());
		if (burned >= 0)
		{
			sim->stop = 1;
			pthread_mutex_unlock(&sim->state_lock);
			log_burnout(sim, burned + 1);
			break ;
		}
		pthread_mutex_unlock(&sim->state_lock);
		usleep(1000);
	}
	return (NULL);
}
