#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

typedef enum e_policy
{
	FIFO,
	EDF
}	t_policy;

typedef struct s_config
{
	int			coder_count;
	long long	burnout;
	long long	compile;
	long long	debug;
	long long	refactor;
	int			required;
	long long	cooldown;
	t_policy	policy;
}	t_config;

int	parse_args(int argc, char **argv, t_config *config);

#endif