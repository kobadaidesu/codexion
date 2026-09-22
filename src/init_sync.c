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

void	destroy_sim_sync(t_sim *sim)
{
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->state_lock);
	pthread_mutex_destroy(&sim->log_lock);
}
