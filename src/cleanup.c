#include "../includes/codexion.h"

void	destroy_dongle_mutexes(t_sim *sim, int count)
{
	while (count > 0)
	{
		count--;
		pthread_mutex_destroy(&sim->dongles[count].lock);
	}
}

void	destroy_sim(t_sim *sim)
{
	destroy_dongle_mutexes(sim, sim->config.number_of_coders);
	destroy_sim_sync(sim);
	free(sim->coders);
	free(sim->dongles);
}
