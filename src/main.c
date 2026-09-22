#include "../includes/codexion.h"

static const char	*error_message(t_error code)
{
	if (code == ERR_ARGC)
		return ("usage: ./codexion number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler");
	if (code == ERR_NOT_A_NUMBER)
		return ("numeric arguments must contain digits only");
	if (code == ERR_OVER_INT_MAX)
		return ("numeric arguments must not exceed INT_MAX");
	if (code == ERR_BAD_SCHEDULER)
		return ("scheduler must be \"fifo\" or \"edf\"");
	if (code == ERR_CODER_COUNT)
		return ("number_of_coders must be at least 1");
	if (code == ERR_MALLOC)
		return ("memory allocation failed");
	if (code == ERR_THREAD)
		return ("thread operation failed");
	if (code == ERR_MUTEX)
		return ("mutex operation failed");
	if (code == ERR_COND)
		return ("condition variable operation failed");
	return ("unknown error");
}

static int	print_error(t_error code)
{
	fprintf(stderr, "codexion: error: %s\n", error_message(code));
	return (EXIT_FAILURE);
}

int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
	t_error		err;

	err = parse_args(argc, argv, &config);
	if (err != SUCCESS)
		return (print_error(err));
	if (config.number_of_compiles_required == 0)
		return (EXIT_SUCCESS);
	err = init_sim(&sim, &config);
	if (err != SUCCESS)
		return (print_error(err));
	err = run_threads(&sim);
	destroy_sim(&sim);
	if (err != SUCCESS)
		return (print_error(err));
	return (EXIT_SUCCESS);
}
