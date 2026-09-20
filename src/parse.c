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

t_error	parse_number(char *str, long long *value)
{
	long long	result;
	int			digit;
	int			i;

	if (!is_number(str))
		return (ERR_NOT_A_NUMBER);
	result = 0;
	i = 0;
	while (str[i])
	{
		digit = str[i] - '0';
		if (result > (INT_MAX - digit) / 10)
			return (ERR_OVER_INT_MAX);
		result = result * 10 + digit;
		i++;
	}
	*value = result;
	return (SUCCESS);
}

t_error	parse_scheduler(char *str, t_scheduler *scheduler)
{
	if (strcmp(str, "fifo") == 0)
		*scheduler = FIFO;
	else if (strcmp(str, "edf") == 0)
		*scheduler = EDF;
	else
		return (ERR_BAD_SCHEDULER);
	return (SUCCESS);
}
