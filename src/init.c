#include "../includes/codexion.h"

static void	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		sim->dongles[i].id = i + 1;
		i++;
	}
}

static void	init_coders(t_sim *sim)
{
	int	i;
	int	count;

	count = sim->config.number_of_coders;
	i = 0;
	while (i < count)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left = &sim->dongles[(i + count - 1) % count];
		sim->coders[i].right = &sim->dongles[i];
		i++;
	}
}

void	destroy_sim(t_sim *sim)
{
	free(sim->coders);
	free(sim->dongles);
}

t_error	init_sim(t_sim *sim, t_config *config)
{
	int	count;

	count = config->number_of_coders;
	sim->config = *config;
	sim->coders = malloc(sizeof(t_coder) * count);
	if (!sim->coders)
		return (ERR_MALLOC);
	sim->dongles = malloc(sizeof(t_dongle) * count);
	if (!sim->dongles)
	{
		free(sim->coders);
		return (ERR_MALLOC);
	}
	init_dongles(sim);
	init_coders(sim);
	return (SUCCESS);
}