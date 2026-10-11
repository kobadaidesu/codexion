#include "../includes/codexion.h"

/*
** dongle.c : 要求登録・待機・取得・解放と時刻
** lock順: dongle->lock → state_lock(逆は全コードで禁止)。
** 待ち行列・holder・ready_atはdongle->lockの中でだけ触る。
** 停止時は要求を残したまま抜ける(停止後は誰も取得しないため無害)。
*/

long long	get_time_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000000 + tv.tv_usec);
}

/* deadline(開始+burnout)は登録時のスナップショットとしてheapに積む */
static void	add_waiter(t_coder *coder, t_dongle *dongle)
{
	long long	deadline;

	pthread_mutex_lock(&coder->sim->state_lock);
	deadline = coder->last_compile_start
		+ coder->sim->config.time_to_burnout * 1000;
	pthread_mutex_unlock(&coder->sim->state_lock);
	pthread_mutex_lock(&dongle->lock);
	heap_push(dongle, coder->id, deadline, coder->sim->config.scheduler);
	pthread_mutex_unlock(&dongle->lock);
}

/* 空きcooldown待ちならready_atで、それ以外は1ms上限で起きる保険付き */
static void	wait_dongle(t_dongle *dongle)
{
	struct timespec	ts;
	long long		wake_us;
	long long		now;

	now = get_time_us();
	wake_us = now + 1000;
	if (dongle->holder == 0 && dongle->ready_at > now)
		wake_us = dongle->ready_at;
	ts.tv_sec = wake_us / 1000000;
	ts.tv_nsec = (wake_us % 1000000) * 1000;
	pthread_cond_timedwait(&dongle->cond, &dongle->lock, &ts);
}

/* 取得条件: heapの先頭が自分 かつ 空き かつ cooldown明け */
int	dongle_take(t_coder *coder, t_dongle *dongle)
{
	t_scheduler	scheduler;

	scheduler = coder->sim->config.scheduler;
	add_waiter(coder, dongle);
	pthread_mutex_lock(&dongle->lock);
	while (!sim_stopped(coder->sim))
	{
		if (heap_top_id(dongle) == coder->id && dongle->holder == 0
			&& get_time_us() >= dongle->ready_at)
		{
			heap_pop(dongle, scheduler);
			dongle->holder = coder->id;
			pthread_mutex_unlock(&dongle->lock);
			return (1);
		}
		wait_dongle(dongle);
	}
	pthread_mutex_unlock(&dongle->lock);
	return (0);
}

void	dongle_release(t_coder *coder, t_dongle *dongle)
{
	long long	now;

	now = get_time_us();
	pthread_mutex_lock(&dongle->lock);
	if (dongle->holder == coder->id)
	{
		dongle->holder = 0;
		dongle->ready_at = now
			+ coder->sim->config.dongle_cooldown * 1000;
		pthread_cond_broadcast(&dongle->cond);
	}
	pthread_mutex_unlock(&dongle->lock);
}
