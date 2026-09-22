#include "../includes/codexion.h"

void	dongle_release(t_coder *coder, t_dongle *dongle)
{
	long long	now;

	now = get_time_us();
	pthread_mutex_lock(&dongle->lock);
	if (dongle->holder == coder->id)
	{
		dongle->holder = 0;
		dongle->ready_at = now
			+ coder->sim->config.dongle_cooldown * 1000;
		pthread_cond_broadcast(&dongle->cond);
	}
	pthread_mutex_unlock(&dongle->lock);
}

void	wake_all_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}
