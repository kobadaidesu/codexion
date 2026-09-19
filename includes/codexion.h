#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <limits.h>

typedef enum e_arg
{
	ARG_NUMBER_OF_CODERS = 1,
	ARG_TIME_TO_BURNOUT,
	ARG_TIME_TO_COMPILE,
	ARG_TIME_TO_DEBUG,
	ARG_TIME_TO_REFACTOR,
	ARG_COMPILES_REQUIRED,
	ARG_DONGLE_COOLDOWN,
	ARG_SCHEDULER
}	t_arg;

typedef enum e_error
{
	SUCCESS = 0,
	ERR_ARGC,
	ERR_NOT_A_NUMBER,
	ERR_OVER_INT_MAX,
	ERR_BAD_SCHEDULER,
	ERR_CODER_COUNT
}	t_error;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_config
{
	int			number_of_coders;
	long long	time_to_burnout;
	long long	time_to_compile;
	long long	time_to_debug;
	long long	time_to_refactor;
	int			number_of_compiles_required;
	long long	dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

t_error	parse_args(int argc, char **argv, t_config *config);
int		print_error(t_error code);

#endif
