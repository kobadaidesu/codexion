#include "../includes/codexion.h"

/*
** coder.c : coderの1周(取得 → compile → debug → refactor)
** dongleはIDの小さい方から取得して循環待ちを防ぐ。
*/

static void	order_dongles(t_coder *coder,
	t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

/* 1人の場合:dongleは1本だけ。compileできず、monitorのburnoutを待つ */
static int	take_single(t_coder *coder)
{
	if (!dongle_take(coder, coder->left))
		return (0);
	log_state(coder, "has taken a dongle");
	while (!sim_stopped(coder->sim))
		usleep(500);
	dongle_release(coder, coder->left);
	return (0);
}

static int	take_pair(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left == coder->right)
		return (take_single(coder));
	order_dongles(coder, &first, &second);
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
	log_state(coder, state);
	return (wait_phase(coder, duration_ms));
}

int	coder_cycle(t_coder *coder)
{
	t_config	*cfg;
	int			ok;

	cfg = &coder->sim->config;
	if (!take_pair(coder))
		return (0);
	ok = begin_compile(coder)
		&& run_phase(coder, "is compiling", cfg->time_to_compile);
	release_pair(coder);
	if (!ok || !finish_compile(coder))
		return (0);
	return (run_phase(coder, "is debugging", cfg->time_to_debug)
		&& run_phase(coder, "is refactoring", cfg->time_to_refactor));
}
