#include "../includes/codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
	t_error		err;

	err = parse_args(argc, argv, &config);
	if (err != SUCCESS)
		return (print_error(err));
	err = init_sim(&sim, &config);
	if (err != SUCCESS)
		return (print_error(err));
	err = run_threads(&sim);
	if (err != SUCCESS)
	{
		destroy_sim(&sim);
		return (print_error(err));
	}
	destroy_sim(&sim);
	return (EXIT_SUCCESS);
}
