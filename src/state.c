#include "../includes/codexion.h"

/*
** state.c : state_lockで守る共有状態(stop/started/compiles/開始時刻)
** _locked付き関数はstate_lock取得済みの文脈からしか呼ばない。
** wake_all_dongles(dongle->lock)はstate_lockを手放してから呼ぶ。
*/

void	start_simulation(t_sim *sim)
{
	long long	now;
	int			i;

	pthread_mutex_lock(&sim->state_lock);
	now = get_time_us();
	sim->start_time = now;
	i = 0;
	while (i < sim->config.number_of_coders)
	{
		sim->coders[i].last_compile_start = now;
		i++;
	}
	sim->started = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->state_lock);
}

int	sim_stopped(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->state_lock);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->state_lock);
	return (stop);
}

int	begin_compile(t_coder *coder)
{
	t_sim		*sim;
	long long	now;
	long long	deadline;

	sim = coder->sim;
	now = get_time_us();
	pthread_mutex_lock(&sim->state_lock);
	deadline = coder_deadline_locked(coder);
	if (sim->stop || now >= deadline)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}
	coder->last_compile_start = now;
	pthread_mutex_unlock(&sim->state_lock);
	return (1);
}

static int	all_completed_locked(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (sim->coders[i].compiles
			< sim->config.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

int	finish_compile(t_coder *coder)
{
	t_sim	*sim;
	int		finished;

	sim = coder->sim;
	finished = 0;
	pthread_mutex_lock(&sim->state_lock);
	coder->compiles++;
	if (all_completed_locked(sim))
	{
		sim->stop = 1;
		finished = 1;
	}
	pthread_mutex_unlock(&sim->state_lock);
	if (finished)
		wake_all_dongles(sim);
	return (!finished);
}
