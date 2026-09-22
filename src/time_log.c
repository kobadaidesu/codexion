#include "../includes/codexion.h"

/*
** time_log.c : 時刻(内部はμs、ログはms)・中断可能な待機・ログ直列化
** log_stateのlock順: state_lock → log_lock。
** coder_deadline_lockedはstate_lock取得済み前提。
*/

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

/* Caller must hold state_lock. Returns absolute burnout deadline in us. */
long long	coder_deadline_locked(t_coder *coder)
{
	return (coder->last_compile_start
		+ coder->sim->config.time_to_burnout * 1000);
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
	pthread_mutex_lock(&sim->log_lock);
	time = get_elapsed_ms(sim);
	printf("%lld %d %s\n", time, coder->id, message);
	pthread_mutex_unlock(&sim->log_lock);
	pthread_mutex_unlock(&sim->state_lock);
}
