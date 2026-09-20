#include "../includes/codexion.h"

static void	destroy_dongle_mutexes(t_sim *sim, int count)
{
	while (count > 0)
	{
		count--;
		pthread_mutex_destroy(&sim->dongles[count].lock);
	}
}

static t_error	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].holder = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
		{
			destroy_dongle_mutexes(sim, i);
			return (ERR_MUTEX);
		}
		i++;
	}
	return (SUCCESS);
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
		sim->coders[i].sim = sim;
		sim->coders[i].left = &sim->dongles[(i + count - 1) % count];
		sim->coders[i].right = &sim->dongles[i];
		i++;
	}
}

void	destroy_sim(t_sim *sim)
{
	destroy_dongle_mutexes(sim, sim->config.number_of_coders);
	free(sim->coders);
	free(sim->dongles);
}

t_error	init_sim(t_sim *sim, t_config *config)
{
	t_error	err;
	int		count;

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
	err = init_dongles(sim);
	if (err != SUCCESS)
	{
		free(sim->coders);
		free(sim->dongles);
		return (err);
	}
	init_coders(sim);
	return (SUCCESS);
}
