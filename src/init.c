#include "../includes/codexion.h"

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
		sim->coders[i].compiles = 0;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].left = &sim->dongles[(i + count - 1) % count];
		sim->coders[i].right = &sim->dongles[i];
		i++;
	}
}

static t_error	alloc_arrays(t_sim *sim, int count)
{
	sim->coders = malloc(sizeof(t_coder) * count);
	if (!sim->coders)
		return (ERR_MALLOC);
	sim->dongles = malloc(sizeof(t_dongle) * count);
	if (!sim->dongles)
	{
		free(sim->coders);
		return (ERR_MALLOC);
	}
	return (SUCCESS);
}

t_error	init_sim(t_sim *sim, t_config *config)
{
	t_error	err;

	sim->config = *config;
	sim->start_time = 0;
	sim->started = 0;
	sim->stop = 0;
	err = alloc_arrays(sim, config->number_of_coders);
	if (err != SUCCESS)
		return (err);
	err = init_dongles(sim);
	if (err == SUCCESS)
	{
		err = init_sim_sync(sim);
		if (err != SUCCESS)
			destroy_dongle_mutexes(sim, config->number_of_coders);
	}
	if (err != SUCCESS)
	{
		free(sim->coders);
		free(sim->dongles);
		return (err);
	}
	init_coders(sim);
	return (SUCCESS);
}
