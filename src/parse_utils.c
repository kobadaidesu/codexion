#include "../includes/codexion.h"

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
