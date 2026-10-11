#include "../includes/codexion.h"

/*
** state.c : state_lockで守る共有状態の遷移と、その監視(monitor)
** lock順: state_lock → log_lock。dongleのcondはstate解放後にのみ触る。
** 期限は常に last_compile_start + time_to_burnout。
*/

int	sim_stopped(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->state_lock);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->state_lock);
	return (stop);
}

/* 期限チェックを時刻更新より先に行う(死者を復活させないため) */
int	begin_compile(t_coder *coder)
{
	t_sim		*sim;
	long long	now;
	long long	deadline;

	sim = coder->sim;
	now = get_time_us();
	pthread_mutex_lock(&sim->state_lock);
	deadline = coder->last_compile_start
		+ sim->config.time_to_burnout * 1000;
	if (sim->stop || now >= deadline)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}
	coder->last_compile_start = now;
	pthread_mutex_unlock(&sim->state_lock);
	return (1);
}

/* 全員がrequired回に達した瞬間、最後の1人がstopを立てる */
int	finish_compile(t_coder *coder)
{
	t_sim	*sim;
	int		done;

	sim = coder->sim;
	done = 0;
	pthread_mutex_lock(&sim->state_lock);
	coder->compiles++;
	if (coder->compiles == sim->config.number_of_compiles_required)
	{
		sim->completed++;
		if (sim->completed == sim->config.number_of_coders)
		{
			sim->stop = 1;
			done = 1;
		}
	}
	pthread_mutex_unlock(&sim->state_lock);
	return (!done);
}

/* state_lock取得済み前提。stop設定とburnoutログを不可分にする */
static int	scan_and_report_locked(t_sim *sim, t_coder *coders)
{
	long long	now;
	int			i;

	now = get_time_us();
	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (coders[i].compiles < sim->config.number_of_compiles_required
			&& now >= coders[i].last_compile_start
			+ sim->config.time_to_burnout * 1000)
		{
			sim->stop = 1;
			pthread_mutex_lock(&sim->log_lock);
			printf("%lld %d burned out\n",
				(now - sim->start_time) / 1000, coders[i].id);
			pthread_mutex_unlock(&sim->log_lock);
			return (1);
		}
		i++;
	}
	return (0);
}

/* 約0.5ms周期で巡回(要件は10ms以内)。終了時に全dongleを起こす */
void	*monitor_routine(void *arg)
{
	t_coder	*coders;
	t_sim	*sim;
	int		i;

	coders = (t_coder *)arg;
	sim = coders[0].sim;
	wait_start(sim);
	pthread_mutex_lock(&sim->state_lock);
	while (!sim->stop && !scan_and_report_locked(sim, coders))
	{
		pthread_mutex_unlock(&sim->state_lock);
		usleep(500);
		pthread_mutex_lock(&sim->state_lock);
	}
	pthread_mutex_unlock(&sim->state_lock);
	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&coders[i].right->lock);
		pthread_cond_broadcast(&coders[i].right->cond);
		pthread_mutex_unlock(&coders[i].right->lock);
		i++;
	}
	return (NULL);
}
