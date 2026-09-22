#include "../includes/codexion.h"

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

static void	release_pair(t_coder *coder)
{
	dongle_release(coder, coder->left);
	dongle_release(coder, coder->right);
}

int	coder_cycle(t_coder *coder)
{
	int	compiled;

	if (!take_pair(coder))
		return (0);
	compiled = begin_compile(coder);
	if (compiled)
	{
		log_state(coder, "is compiling");
		compiled = wait_phase(coder, coder->sim->config.time_to_compile);
	}
	release_pair(coder);
	if (!compiled || !finish_compile(coder))
		return (0);
	log_state(coder, "is debugging");
	if (!wait_phase(coder, coder->sim->config.time_to_debug))
		return (0);
	log_state(coder, "is refactoring");
	return (wait_phase(coder, coder->sim->config.time_to_refactor));
}
