#include "../includes/codexion.h"

static void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (coder->compiles < coder->sim->config.number_of_compiles_required)
	{
		if (!coder_cycle(coder))
			break ;
	}
	return (NULL);
}

static int	join_threads(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}

t_error	run_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			join_threads(sim, i);
			return (ERR_THREAD);
		}
		i++;
	}
	if (join_threads(sim, i))
		return (ERR_THREAD);
	return (SUCCESS);
}
