#include "../includes/codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;

	if (parse_args(argc, argv, &config) != 0)
	{
		printf("Error\n");
		return (1);
	}
	printf("coders: %d\n", config.coder_count);
	printf("burnout: %lld\n", config.burnout);
	printf("compile: %lld\n", config.compile);
	printf("scheduler: %s\n", argv[8]);
	return (0);
}