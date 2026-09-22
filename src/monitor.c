#include "../includes/codexion.h"

static void	wait_monitor_start(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_lock);
	while (!sim->started && !sim->stop)
		pthread_cond_wait(&sim->start_cond, &sim->state_lock);
	pthread_mutex_unlock(&sim->state_lock);
}

static int	find_burned_locked(t_sim *sim, long long now)
{
	long long	deadline;
	int			i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (sim->coders[i].compiles
			< sim->config.number_of_compiles_required)
		{
			deadline = coder_deadline_locked(&sim->coders[i]);
			if (now >= deadline)
				return (i);
		}
		i++;
	}
	return (-1);
}

/* state_lock取得済み前提。stop設定とburnoutログを不可分にする */
static void	report_burnout(t_sim *sim, int index)
{
	long long	time;

	sim->stop = 1;
	pthread_mutex_lock(&sim->log_lock);
	time = get_elapsed_ms(sim);
	printf("%lld %d burned out\n", time, sim->coders[index].id);
	pthread_mutex_unlock(&sim->log_lock);
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
		burned = find_burned_locked(sim, get_time_us());
		if (burned >= 0)
		{
			report_burnout(sim, burned);
			pthread_mutex_unlock(&sim->state_lock);
			wake_all_dongles(sim);
			break ;
		}
		pthread_mutex_unlock(&sim->state_lock);
		usleep(500);
	}
	return (NULL);
}
