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

static int	take_pair(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left == coder->right)
		return (0);
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

static int	debug_and_refactor(t_coder *coder)
{
	log_state(coder, "is debugging");
	if (!wait_phase(coder, coder->sim->config.time_to_debug))
		return (0);
	log_state(coder, "is refactoring");
	if (!wait_phase(coder, coder->sim->config.time_to_refactor))
		return (0);
	return (1);
}

int	coder_cycle(t_coder *coder)
{
	if (!take_pair(coder))
		return (0);
	if (!begin_compile(coder))
	{
		release_pair(coder);
		return (0);
	}
	log_state(coder, "is compiling");
	if (!wait_phase(coder, coder->sim->config.time_to_compile))
	{
		release_pair(coder);
		return (0);
	}
	release_pair(coder);
	if (!finish_compile(coder))
		return (0);
	return (debug_and_refactor(coder));
}
