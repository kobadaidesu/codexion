#include "../includes/codexion.h"

void	log_state(t_coder *coder, char *message)
{
	long long	time;

	pthread_mutex_lock(&coder->sim->log_lock);
	time = get_elapsed_ms(coder->sim);
	printf("%lld %d %s\n", time, coder->id, message);
	pthread_mutex_unlock(&coder->sim->log_lock);
}
