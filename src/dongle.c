#include "../includes/codexion.h"

int	dongle_try_take(t_coder *coder, t_dongle *dongle)
{
	int	taken;

	taken = 0;
	pthread_mutex_lock(&dongle->lock);
	if (dongle->holder == 0)
	{
		dongle->holder = coder->id;
		taken = 1;
	}
	pthread_mutex_unlock(&dongle->lock);
	return (taken);
}

void	dongle_release(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);
	if (dongle->holder == coder->id)
		dongle->holder = 0;
	pthread_mutex_unlock(&dongle->lock);
}
