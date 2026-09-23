#include "../includes/codexion.h"

/*
** state.c : state_lockで守る共有状態(stop/started/completed/開始時刻)
** _locked付き関数はstate_lock取得済みの文脈からしか呼ばない。
*/

void	start_simulation(t_sim *sim, t_coder *coders)
{
	long long	now;
	int			i;

	pthread_mutex_lock(&sim->state_lock);
	now = get_time_us();
	sim->start_time = now;
	i = 0;
	while (i < sim->config.number_of_coders)
	{
		coders[i].last_compile_start = now;
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
