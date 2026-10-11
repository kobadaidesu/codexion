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

typedef enum e_error
{
	SUCCESS = 0,
	ERR_ARGS,
	ERR_FATAL
}	t_error;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

/* ① 設定: 起動時に確定し、以後は読み取り専用 */
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

/* ② 全threadが共有する実行状態。可変メンバはstate_lockで守る */
typedef struct s_sim
{
	t_config		config;
	long long		start_time;
	int				started;
	int				stop;
	int				completed;
	pthread_mutex_t	state_lock;
	pthread_mutex_t	log_lock;
	pthread_cond_t	start_cond;
}	t_sim;

/*
** ③ 共有資源。待ち行列は登録時のスナップショット(id/ticket/deadline)
** を並行配列で持つ二分ヒープ。coder型には依存しない(IDのみ保存)。
** 全メンバはlockの中でだけ触る。
*/
typedef struct s_dongle
{
	int				id;
	int				holder;
	long long		ready_at;
	long long		next_ticket;
	int				wait_count;
	int				wait_ids[DONGLE_MAX_WAITERS];
	long long		wait_tickets[DONGLE_MAX_WAITERS];
	long long		wait_deadlines[DONGLE_MAX_WAITERS];
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

/* ④ 各thread。sim/left/rightはすべて参照(所有はmain側) */
typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	int			compiles;
	long long	last_compile_start;
	t_sim		*sim;
	t_dongle	*left;
	t_dongle	*right;
}	t_coder;

/* main.c : 引数検証・入口・エラー表示 */
/* init.c : 構築(メモリ・同期プリミティブ)と後片付け */
t_error		init_sim(t_sim *sim, t_config *config,
				t_coder **coders, t_dongle **dongles);
void		destroy_sim(t_sim *sim, t_coder *coders, t_dongle *dongles);

/* thread.c : thread生成・一斉スタートゲート・join */
t_error		run_threads(t_sim *sim, t_coder *coders);
int			wait_start(t_sim *sim);

/* coder.c : coderスレッドの一生(取得→compile→debug→refactor) */
void		*coder_routine(void *arg);

/* dongle.c : 要求登録・待機・取得・解放と時刻 */
long long	get_time_us(void);
int			dongle_take(t_coder *coder, t_dongle *dongle);
void		dongle_release(t_coder *coder, t_dongle *dongle);

/* heap.c : dongle待ち行列(優先度付きキュー)。FIFO/EDF比較も内包 */
void		heap_push(t_dongle *dongle, int coder_id,
				long long deadline, t_scheduler scheduler);
int			heap_top_id(t_dongle *dongle);
void		heap_pop(t_dongle *dongle, t_scheduler scheduler);

/* state.c : 共有状態の遷移(開始・停止・compile記録)と監視thread */
int			sim_stopped(t_sim *sim);
int			begin_compile(t_coder *coder);
int			finish_compile(t_coder *coder);
void		*monitor_routine(void *arg);

#endif
