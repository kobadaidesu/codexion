#include "../includes/codexion.h"

static void	destroy_dongle_sync(t_dongle *dongles, int count)
{
	while (count > 0)
	{
		count--;
		pthread_cond_destroy(&dongles[count].cond);
		pthread_mutex_destroy(&dongles[count].lock);
	}
}

/* 所有者であるmainが後片付けも持つ */
static void	destroy_sim(t_sim *sim, t_coder *coders, t_dongle *dongles)
{
	destroy_dongle_sync(dongles, sim->config.number_of_coders);
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->state_lock);
	pthread_mutex_destroy(&sim->log_lock);
	free(coders);
	free(dongles);
}

static int	print_error(t_error code)
{
	if (code == ERR_ARGS)
		fprintf(stderr, "codexion: usage: ./codexion number_of_coders "
			"time_to_burnout time_to_compile time_to_debug "
			"time_to_refactor number_of_compiles_required "
			"dongle_cooldown scheduler(fifo|edf)\n");
	else
		fprintf(stderr, "codexion: fatal error\n");
	return (EXIT_FAILURE);
}

int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
	t_coder		*coders;
	t_dongle	*dongles;
	t_error		err;

	err = parse_args(argc, argv, &config);
	if (err != SUCCESS)
		return (print_error(err));
	if (config.number_of_compiles_required == 0)
		return (EXIT_SUCCESS);
	err = init_sim(&sim, &config, &coders, &dongles);
	if (err != SUCCESS)
		return (print_error(err));
	err = run_threads(&sim, coders);
	destroy_sim(&sim, coders, dongles);
	if (err != SUCCESS)
		return (print_error(err));
	return (EXIT_SUCCESS);
}
