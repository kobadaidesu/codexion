#include "../includes/codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;
	t_sim		sim;
	t_error		err;
	int			i;

	err = parse_args(argc, argv, &config);
	if (err != SUCCESS)
		return (print_error(err));
	err = init_sim(&sim, &config);
	if (err != SUCCESS)
		return (print_error(err));
	i = 0;
	while (i < config.number_of_coders)
	{
		printf("coder %d: left=%d right=%d\n",
			sim.coders[i].id,
			sim.coders[i].left->id,
			sim.coders[i].right->id);
		i++;
	}
	destroy_sim(&sim);
	return (EXIT_SUCCESS);
}