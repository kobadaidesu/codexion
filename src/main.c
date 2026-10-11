#include "../includes/codexion.h"

static t_error	parse_number(const char *str, long long *value)
{
	long long	result;
	int			digit;

	if (!str || !*str)
		return (ERR_ARGS);
	result = 0;
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (ERR_ARGS);
		digit = *str - '0';
		if (result > (INT_MAX - digit) / 10)
			return (ERR_ARGS);
		result = result * 10 + digit;
		str++;
	}
	*value = result;
	return (SUCCESS);
}

static void	assign_config(t_config *config, long long *values)
{
	config->number_of_coders = (int)values[0];
	config->time_to_burnout = values[1];
	config->time_to_compile = values[2];
	config->time_to_debug = values[3];
	config->time_to_refactor = values[4];
	config->number_of_compiles_required = (int)values[5];
	config->dongle_cooldown = values[6];
}

static t_error	parse_args(int argc, char **argv, t_config *config)
{
	long long	values[7];
	int			i;

	if (argc != 9)
		return (ERR_ARGS);
	i = 0;
	while (i < 7)
	{
		if (parse_number(argv[i + 1], &values[i]) != SUCCESS)
			return (ERR_ARGS);
		i++;
	}
	if (!strcmp(argv[8], "fifo"))
		config->scheduler = FIFO;
	else if (!strcmp(argv[8], "edf"))
		config->scheduler = EDF;
	else
		return (ERR_ARGS);
	assign_config(config, values);
	if (config->number_of_coders < 1)
		return (ERR_ARGS);
	return (SUCCESS);
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
