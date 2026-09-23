#include "../includes/codexion.h"

static t_error	init_one_dongle(t_dongle *dongle, int id)
{
	dongle->id = id;
	dongle->holder = 0;
	dongle->ready_at = 0;
	dongle->next_ticket = 0;
	dongle->wait_count = 0;
	if (pthread_mutex_init(&dongle->lock, NULL) != 0)
		return (ERR_MUTEX);
	if (pthread_cond_init(&dongle->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&dongle->lock);
		return (ERR_COND);
	}
	return (SUCCESS);
}

static t_error	init_dongles(t_dongle *dongles, int count)
{
	int		i;
	t_error	err;

	i = 0;
	while (i < count)
	{
		err = init_one_dongle(&dongles[i], i + 1);
		if (err != SUCCESS)
		{
			destroy_dongle_sync(dongles, i);
			return (err);
		}
		i++;
	}
	return (SUCCESS);
}

static void	init_coders(t_sim *sim, t_coder *coders, t_dongle *dongles)
{
	int	i;
	int	count;

	count = sim->config.number_of_coders;
	i = 0;
	while (i < count)
	{
		coders[i].id = i + 1;
		coders[i].sim = sim;
		coders[i].compiles = 0;
		coders[i].last_compile_start = 0;
		coders[i].left = &dongles[(i + count - 1) % count];
		coders[i].right = &dongles[i];
		i++;
	}
}

static t_error	alloc_arrays(t_coder **coders, t_dongle **dongles, int count)
{
	*coders = malloc(sizeof(t_coder) * count);
	if (!*coders)
		return (ERR_MALLOC);
	*dongles = malloc(sizeof(t_dongle) * count);
	if (!*dongles)
	{
		free(*coders);
		return (ERR_MALLOC);
	}
	return (SUCCESS);
}

t_error	init_sim(t_sim *sim, t_config *config,
	t_coder **coders, t_dongle **dongles)
{
	t_error	err;

	sim->config = *config;
	sim->start_time = 0;
	sim->started = 0;
	sim->stop = 0;
	sim->completed = 0;
	err = alloc_arrays(coders, dongles, config->number_of_coders);
	if (err != SUCCESS)
		return (err);
	err = init_dongles(*dongles, config->number_of_coders);
	if (err == SUCCESS)
	{
		err = init_sim_sync(sim);
		if (err != SUCCESS)
			destroy_dongle_sync(*dongles, config->number_of_coders);
	}
	if (err != SUCCESS)
	{
		free(*coders);
		free(*dongles);
		return (err);
	}
	init_coders(sim, *coders, *dongles);
	return (SUCCESS);
}
