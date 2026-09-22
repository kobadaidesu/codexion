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

int	wait_phase(t_coder *coder, long long duration_ms)
{
	long long	end;

	end = get_time_us() + duration_ms * 1000;
	while (get_time_us() < end)
	{
		if (sim_stopped(coder->sim))
			return (0);
		usleep(500);
	}
	return (1);
}
