#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <limits.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

# define DONGLE_MAX_WAITERS 2

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
	ERR_CODER_COUNT,
	ERR_MALLOC,
	ERR_THREAD,
	ERR_MUTEX,
	ERR_COND
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

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;

typedef struct s_waiter
{
	t_coder		*coder;
	long long	ticket;
	long long	deadline;
}	t_waiter;

typedef struct s_heap
{
	t_waiter	data[DONGLE_MAX_WAITERS];
	int			size;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				holder;
	long long		ready_at;
	long long		next_ticket;
	t_heap			waiters;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

struct s_coder
{
	int			id;
	pthread_t	thread;
	t_sim		*sim;
	t_dongle	*left;
	t_dongle	*right;
	int			compiles;
	long long	last_compile_start;
};

struct s_sim
{
	t_config		config;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
	long long		start_time;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
	pthread_cond_t	start_cond;
	int				started;
	int				stop;
};

t_error		parse_args(int argc, char **argv, t_config *config);
t_error		init_sim(t_sim *sim, t_config *config);
void		destroy_sim(t_sim *sim);
t_error		run_threads(t_sim *sim);
int			dongle_take(t_coder *coder, t_dongle *dongle);
void		dongle_release(t_coder *coder, t_dongle *dongle);
void		wake_all_dongles(t_sim *sim);
void		heap_push(t_heap *heap, t_waiter waiter, t_scheduler scheduler);
void		heap_pop(t_heap *heap, t_scheduler scheduler);
t_waiter	*heap_top(t_heap *heap);
void		heap_remove_coder(t_heap *heap, t_coder *coder,
				t_scheduler scheduler);
void		heap_up(t_heap *heap, int index, t_scheduler scheduler);
void		heap_down(t_heap *heap, int index, t_scheduler scheduler);
void		heap_fix(t_heap *heap, int index, t_scheduler scheduler);
int			coder_cycle(t_coder *coder);
long long	get_time_us(void);
long long	get_elapsed_ms(t_sim *sim);
int			wait_phase(t_coder *coder, long long duration_ms);
void		log_state(t_coder *coder, char *message);
void		start_simulation(t_sim *sim);
int			sim_stopped(t_sim *sim);
int			begin_compile(t_coder *coder);
int			finish_compile(t_coder *coder);
void		*monitor_routine(void *arg);
t_error		init_sim_sync(t_sim *sim);
void		destroy_dongle_sync(t_sim *sim, int count);

#endif
