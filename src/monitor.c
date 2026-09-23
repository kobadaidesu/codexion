#include "../includes/codexion.h"

/*
** monitor.c : burnout監視thread(約0.5ms周期、要件は10ms以内の検知)
** 引数はcoders配列。simへはcoders[0].simで辿る。
** lock順: state_lock → log_lock。
** stopの理由が何であれ、終了時に全dongleを起こしてから抜ける。
*/

static int	find_burned_locked(t_sim *sim, t_coder *coders, long long now)
{
	long long	deadline;
	int			i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (coders[i].compiles < sim->config.number_of_compiles_required)
		{
			deadline = coder_deadline_locked(&coders[i]);
			if (now >= deadline)
				return (i);
		}
		i++;
	}
	return (-1);
}

/* state_lock取得済み前提。stop設定とburnoutログを不可分にする */
static void	report_burnout(t_sim *sim, int coder_id)
{
	long long	time;

	sim->stop = 1;
	pthread_mutex_lock(&sim->log_lock);
	time = get_elapsed_ms(sim);
	printf("%lld %d burned out\n", time, coder_id);
	pthread_mutex_unlock(&sim->log_lock);
}

void	*monitor_routine(void *arg)
{
	t_coder	*coders;
	t_sim	*sim;
	int		burned;

	coders = (t_coder *)arg;
	sim = coders[0].sim;
	wait_start(sim);
	while (1)
	{
		pthread_mutex_lock(&sim->state_lock);
		if (sim->stop)
		{
			pthread_mutex_unlock(&sim->state_lock);
			break ;
		}
		burned = find_burned_locked(sim, coders, get_time_us());
		if (burned >= 0)
		{
			report_burnout(sim, coders[burned].id);
			pthread_mutex_unlock(&sim->state_lock);
			break ;
		}
		pthread_mutex_unlock(&sim->state_lock);
		usleep(500);
	}
	wake_all_dongles(coders, sim->config.number_of_coders);
	return (NULL);
}
