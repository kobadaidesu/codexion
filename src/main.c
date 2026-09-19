#include "../includes/codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;
	t_error		err;

	err = parse_args(argc, argv, &config);
	if (err != SUCCESS)
		return (print_error(err));
	printf("coders: %d\n", config.number_of_coders);
	printf("burnout: %lld\n", config.time_to_burnout);
	printf("compile: %lld\n", config.time_to_compile);
	printf("scheduler: %s\n", argv[ARG_SCHEDULER]);
	return (EXIT_SUCCESS);
}
