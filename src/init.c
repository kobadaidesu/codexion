#include "../includes/codexion.h"

static t_error	init_one_dongle(t_dongle *dongle, int id)
{
	dongle->id = id;
	dongle->holder = 0;
	dongle->ready_at = 0;
	dongle->next_ticket = 0;
	dongle->waiters.size = 0;
	if (pthread_mutex_init(&dongle->lock, NULL) != 0)
		return (ERR_MUTEX);
	if (pthread_cond_init(&dongle->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&dongle->lock);
		return (ERR_COND);
	}
	return (SUCCESS);
}

static t_error	init_dongles(t_sim *sim)
{
	int		i;
	t_error	err;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		err = init_one_dongle(&sim->dongles[i], i + 1);
		if (err != SUCCESS)
		{
			destroy_dongle_sync(sim, i);
			return (err);
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
			destroy_dongle_sync(sim, config->number_of_coders);
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
