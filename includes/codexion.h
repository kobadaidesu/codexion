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

/* 起動時に確定し、以後は読み取り専用の設定値 */
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

/*
** 待ち行列は登録時のスナップショット(id/ticket/deadline)を
** 並行配列で持つ二分ヒープ。全メンバはlockの中でだけ触る。
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

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	int			compiles;
	long long	last_compile_start;
	t_dongle	*left;
	t_dongle	*right;
	t_sim		*sim;
}	t_coder;

/* 全体の器。可変メンバ(stop等)はstate_lockで守る */
struct s_sim
{
	t_config		config;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
	long long		start_time;
	int				started;
	int				stop;
	int				completed;
	pthread_mutex_t	state_lock;
	pthread_mutex_t	log_lock;
	pthread_cond_t	start_cond;
};

/* parse.c : 引数検証・数値変換 */
t_error		parse_args(int argc, char **argv, t_config *config);

/* init.c : メモリ確保・coderとdongleの対応付け */
t_error		init_sim(t_sim *sim, t_config *config);

/* sync.c : mutex/condの生成と破棄・後片付け */
t_error		init_sim_sync(t_sim *sim);
void		destroy_dongle_sync(t_sim *sim, int count);
void		destroy_sim(t_sim *sim);

/* thread.c : thread生成・開始ゲート・join */
t_error		run_threads(t_sim *sim);
int			wait_start(t_sim *sim);

/* coder.c : coderの1周(取得→compile→debug→refactor) */
int			coder_cycle(t_coder *coder);

/* dongle_take.c : heap登録と条件変数待ちによる取得 */
int			dongle_take(t_coder *coder, t_dongle *dongle);

/* dongle_release.c : 解放・cooldown設定・待機threadの起床 */
void		dongle_release(t_coder *coder, t_dongle *dongle);
void		release_pair(t_coder *coder);
void		wake_all_dongles(t_sim *sim);

/* heap.c : dongle待ち行列(優先度付きキュー)の追加・取り出し・削除 */
void		heap_push(t_dongle *dongle, int coder_id,
				long long deadline, t_scheduler scheduler);
int			heap_top_id(t_dongle *dongle);
void		heap_pop(t_dongle *dongle, t_scheduler scheduler);
void		heap_remove(t_dongle *dongle, int coder_id,
				t_scheduler scheduler);

/* heap_order.c : FIFO/EDF比較とヒープの並び直し */
void		heap_up(t_dongle *dongle, int index, t_scheduler scheduler);
void		heap_down(t_dongle *dongle, int index, t_scheduler scheduler);
void		heap_fix(t_dongle *dongle, int index, t_scheduler scheduler);

/* state.c : state_lockで守る状態遷移(開始・停止・compile記録) */
void		start_simulation(t_sim *sim);
int			sim_stopped(t_sim *sim);
int			begin_compile(t_coder *coder);
int			finish_compile(t_coder *coder);

/* monitor.c : burnout監視。停止理由を問わず終了時に全dongleを起こす */
void		*monitor_routine(void *arg);

/* time_log.c : 時刻取得・中断可能な待機・ログ直列化・期限計算 */
long long	get_time_us(void);
long long	get_elapsed_ms(t_sim *sim);
long long	coder_deadline_locked(t_coder *coder);
int			wait_phase(t_coder *coder, long long duration_ms);
void		log_state(t_coder *coder, char *message);

#endif
