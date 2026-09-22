#include "../includes/codexion.h"

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
