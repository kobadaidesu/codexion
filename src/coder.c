#include "../includes/codexion.h"

/*
** coder.c : coderスレッドの一生(取得 → compile → debug → refactor)
** dongleはIDの小さい方から取得して循環待ちを防ぐ。
** ログのlock順: state_lock → log_lock。stop後は何も出力しない。
*/

static void	log_state(t_coder *coder, char *message)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	if (sim->stop)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return ;
	}
	pthread_mutex_lock(&sim->log_lock);
	printf("%lld %d %s\n", (get_time_us() - sim->start_time) / 1000,
		coder->id, message);
	pthread_mutex_unlock(&sim->log_lock);
	pthread_mutex_unlock(&sim->state_lock);
}

static int	take_pair(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left;
	second = coder->right;
	if (second->id < first->id)
	{
		first = coder->right;
		second = coder->left;
	}
	if (!dongle_take(coder, first))
		return (0);
	log_state(coder, "has taken a dongle");
	if (!dongle_take(coder, second))
	{
		dongle_release(coder, first);
		return (0);
	}
	log_state(coder, "has taken a dongle");
	return (1);
}

static int	run_phase(t_coder *coder, char *state, long long duration_ms)
{
	long long	end;

	log_state(coder, state);
	end = get_time_us() + duration_ms * 1000;
	while (get_time_us() < end)
	{
		if (sim_stopped(coder->sim))
			return (0);
		usleep(500);
	}
	return (1);
}

static int	coder_cycle(t_coder *coder)
{
	t_config	*cfg;
	int			ok;

	cfg = &coder->sim->config;
	if (!take_pair(coder))
		return (0);
	ok = begin_compile(coder)
		&& run_phase(coder, "is compiling", cfg->time_to_compile);
	dongle_release(coder, coder->left);
	dongle_release(coder, coder->right);
	if (!ok || !finish_compile(coder))
		return (0);
	return (run_phase(coder, "is debugging", cfg->time_to_debug)
		&& run_phase(coder, "is refactoring", cfg->time_to_refactor));
}

/* 1人の場合はdongle1本を取り、monitorのburnoutを待つ */
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (!wait_start(coder->sim))
		return (NULL);
	if (coder->left == coder->right)
	{
		if (dongle_take(coder, coder->left))
			log_state(coder, "has taken a dongle");
		while (!sim_stopped(coder->sim))
			usleep(500);
		dongle_release(coder, coder->left);
		return (NULL);
	}
	while (coder->compiles < coder->sim->config.number_of_compiles_required)
	{
		if (!coder_cycle(coder))
			break ;
	}
	return (NULL);
}
