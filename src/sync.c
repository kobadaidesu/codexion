#include "../includes/codexion.h"

t_error	init_sim_sync(t_sim *sim)
{
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0)
		return (ERR_MUTEX);
	if (pthread_mutex_init(&sim->state_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_lock);
		return (ERR_MUTEX);
	}
	if (pthread_cond_init(&sim->start_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->state_lock);
		pthread_mutex_destroy(&sim->log_lock);
		return (ERR_COND);
	}
	return (SUCCESS);
}

static void	destroy_sim_sync(t_sim *sim)
{
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->state_lock);
	pthread_mutex_destroy(&sim->log_lock);
}

void	destroy_dongle_sync(t_dongle *dongles, int count)
{
	while (count > 0)
	{
		count--;
		pthread_cond_destroy(&dongles[count].cond);
		pthread_mutex_destroy(&dongles[count].lock);
	}
}

void	destroy_sim(t_sim *sim, t_coder *coders, t_dongle *dongles)
{
	destroy_dongle_sync(dongles, sim->config.number_of_coders);
	destroy_sim_sync(sim);
	free(coders);
	free(dongles);
}
