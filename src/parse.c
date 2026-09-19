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
	int			i;
	long long	result;

	if (!is_number(str))
		return (ERR_NOT_A_NUMBER);
	i = 0;
	result = 0;
	while (str[i])
	{
		result = result * 10 + (str[i] - '0');
		if (result > INT_MAX)
			return (ERR_OVER_INT_MAX);
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
		return (ERR_BAD_SCHEDULER);
	return (SUCCESS);
}

static t_error	parse_second(char **argv, t_config *config)
{
	long long	value;
	t_error		err;

	err = parse_number(argv[ARG_TIME_TO_REFACTOR],
			&config->time_to_refactor);
	if (err != SUCCESS)
		return (err);
	err = parse_number(argv[ARG_COMPILES_REQUIRED], &value);
	if (err != SUCCESS)
		return (err);
	config->number_of_compiles_required = (int)value;
	err = parse_number(argv[ARG_DONGLE_COOLDOWN], &config->dongle_cooldown);
	if (err != SUCCESS)
		return (err);
	return (parse_scheduler(argv[ARG_SCHEDULER], &config->scheduler));
}

static t_error	parse_first(char **argv, t_config *config)
{
	long long	value;
	t_error		err;

	err = parse_number(argv[ARG_NUMBER_OF_CODERS], &value);
	if (err != SUCCESS)
		return (err);
	config->number_of_coders = (int)value;
	err = parse_number(argv[ARG_TIME_TO_BURNOUT], &config->time_to_burnout);
	if (err != SUCCESS)
		return (err);
	err = parse_number(argv[ARG_TIME_TO_COMPILE], &config->time_to_compile);
	if (err != SUCCESS)
		return (err);
	err = parse_number(argv[ARG_TIME_TO_DEBUG], &config->time_to_debug);
	if (err != SUCCESS)
		return (err);
	return (SUCCESS);
}

t_error	parse_args(int argc, char **argv, t_config *config)
{
	t_error	err;

	if (argc != 9)
		return (ERR_ARGC);
	err = parse_first(argv, config);
	if (err != SUCCESS)
		return (err);
	err = parse_second(argv, config);
	if (err != SUCCESS)
		return (err);
	if (config->number_of_coders < 1)
		return (ERR_CODER_COUNT);
	return (SUCCESS);
}
