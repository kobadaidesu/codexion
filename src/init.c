#include "../includes/codexion.h"

static t_error	init_sim_sync(t_sim *sim)
{
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0
		|| pthread_mutex_init(&sim->state_lock, NULL) != 0
		|| pthread_cond_init(&sim->start_cond, NULL) != 0)
		return (ERR_FATAL);
	return (SUCCESS);
}

static t_error	init_dongles(t_dongle *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		dongles[i].id = i + 1;
		dongles[i].holder = 0;
		dongles[i].ready_at = 0;
		dongles[i].next_ticket = 0;
		dongles[i].wait_count = 0;
		if (pthread_mutex_init(&dongles[i].lock, NULL) != 0
			|| pthread_cond_init(&dongles[i].cond, NULL) != 0)
			return (ERR_FATAL);
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
		return (ERR_FATAL);
	*dongles = malloc(sizeof(t_dongle) * count);
	if (!*dongles)
	{
		free(*coders);
		return (ERR_FATAL);
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
		err = init_sim_sync(sim);
	if (err != SUCCESS)
	{
		free(*coders);
		free(*dongles);
		return (err);
	}
	init_coders(sim, *coders, *dongles);
	return (SUCCESS);
}
