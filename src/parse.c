#include "../includes/codexion.h"

static int	is_number(char *str)
{
	int	i;

	if (!str || !str[0])
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static t_error	parse_number(char *str, long long *value)
{
	long long	result;
	int			digit;
	int			i;

	if (!is_number(str))
		return (ERR_ARGS);
	result = 0;
	i = 0;
	while (str[i])
	{
		digit = str[i] - '0';
		if (result > (INT_MAX - digit) / 10)
			return (ERR_ARGS);
		result = result * 10 + digit;
		i++;
	}
	*value = result;
	return (SUCCESS);
}

static t_error	parse_scheduler(char *str, t_scheduler *scheduler)
{
	if (strcmp(str, "fifo") == 0)
		*scheduler = FIFO;
	else if (strcmp(str, "edf") == 0)
		*scheduler = EDF;
	else
		return (ERR_ARGS);
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

t_error	parse_args(int argc, char **argv, t_config *config)
{
	long long	values[7];
	t_error		err;
	int			i;

	if (argc != 9)
		return (ERR_ARGS);
	i = 0;
	while (i < 7)
	{
		err = parse_number(argv[i + 1], &values[i]);
		if (err != SUCCESS)
			return (err);
		i++;
	}
	err = parse_scheduler(argv[8], &config->scheduler);
	if (err != SUCCESS)
		return (err);
	assign_config(config, values);
	if (config->number_of_coders < 1)
		return (ERR_ARGS);
	return (SUCCESS);
}
