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

static int	parse_number(char *str, long long *value)
{
	int			i;
	long long	result;

	if (!is_number(str))
		return (1);
	i = 0;
	result = 0;
	while (str[i])
	{
		result = result * 10 + (str[i] - '0');
		if (result > 2147483647)
			return (1);
		i++;
	}
	*value = result;
	return (0);
}

static int	parse_policy(char *str, t_policy *policy)
{
	if (strcmp(str, "fifo") == 0)
		*policy = FIFO;
	else if (strcmp(str, "edf") == 0)
		*policy = EDF;
	else
		return (1);
	return (0);
}

static int	parse_second(char **argv, t_config *config)
{
	long long	value;

	if (parse_number(argv[5], &config->refactor))
		return (1);
	if (parse_number(argv[6], &value))
		return (1);
	config->required = (int)value;
	if (parse_number(argv[7], &config->cooldown))
		return (1);
	if (parse_policy(argv[8], &config->policy))
		return (1);
	return (0);
}

static int	parse_first(char **argv, t_config *config)
{
	long long	value;

	if (parse_number(argv[1], &value))
		return (1);
	config->coder_count = (int)value;
	if (parse_number(argv[2], &config->burnout))
		return (1);
	if (parse_number(argv[3], &config->compile))
		return (1);
	if (parse_number(argv[4], &config->debug))
		return (1);
	return (0);
}

int	parse_args(int argc, char **argv, t_config *config)
{
	if (argc != 9)
		return (1);
	if (parse_first(argv, config))
		return (1);
	if (parse_second(argv, config))
		return (1);
	if (config->coder_count < 1)
		return (1);
	return (0);
}