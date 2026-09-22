#include "../includes/codexion.h"

long long	get_time_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000000 + tv.tv_usec);
}

long long	get_elapsed_ms(t_sim *sim)
{
	long long	now;

	now = get_time_us();
	return ((now - sim->start_time) / 1000);
}
