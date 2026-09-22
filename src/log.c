#include "../includes/codexion.h"

void	log_state(t_coder *coder, char *message)
{
	t_sim		*sim;
	long long	time;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	if (sim->stop)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return ;
	}
	time = get_elapsed_ms(sim);
	pthread_mutex_lock(&sim->log_lock);
	printf("%lld %d %s\n", time, coder->id, message);
	pthread_mutex_unlock(&sim->log_lock);
	pthread_mutex_unlock(&sim->state_lock);
}

void	log_burnout(t_sim *sim, int coder_id)
{
	long long	time;

	pthread_mutex_lock(&sim->log_lock);
	time = get_elapsed_ms(sim);
	printf("%lld %d burned out\n", time, coder_id);
	pthread_mutex_unlock(&sim->log_lock);
}
