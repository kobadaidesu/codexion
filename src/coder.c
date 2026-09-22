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
	dongle_take(coder, first);
	log_state(coder, "has taken a dongle");
	dongle_take(coder, second);
	log_state(coder, "has taken a dongle");
	return (1);
}

static void	compile_code(t_coder *coder)
{
	log_state(coder, "is compiling");
	usleep(coder->sim->config.time_to_compile * 1000);
	dongle_release(coder, coder->left);
	dongle_release(coder, coder->right);
	coder->compiles++;
}

static void	debug_and_refactor(t_coder *coder)
{
	log_state(coder, "is debugging");
	usleep(coder->sim->config.time_to_debug * 1000);
	log_state(coder, "is refactoring");
	usleep(coder->sim->config.time_to_refactor * 1000);
}

int	coder_cycle(t_coder *coder)
{
	if (!take_pair(coder))
		return (0);
	compile_code(coder);
	debug_and_refactor(coder);
	return (1);
}
