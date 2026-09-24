#include "../includes/codexion.h"

/*
** dongle_take.c : 要求登録・待機・取得
** lock順: dongle->lock → state_lock(逆は全コードで禁止)。
** 待ち行列・holder・ticket・ready_atはdongle->lockの中でだけ触る。
** 停止時は要求を残したまま抜ける(停止後は誰も取得しないため無害)。
*/

static long long	get_coder_deadline(t_coder *coder)
{
	long long	deadline;

	pthread_mutex_lock(&coder->sim->state_lock);
	deadline = coder_deadline_locked(coder);
	pthread_mutex_unlock(&coder->sim->state_lock);
	return (deadline);
}

static void	add_waiter(t_coder *coder, t_dongle *dongle)
{
	long long	deadline;

	deadline = get_coder_deadline(coder);
	pthread_mutex_lock(&dongle->lock);
	heap_push(dongle, coder->id, deadline, coder->sim->config.scheduler);
	pthread_mutex_unlock(&dongle->lock);
}

static int	can_grant(t_coder *coder, t_dongle *dongle)
{
	if (heap_top_id(dongle) != coder->id)
		return (0);
	if (dongle->holder != 0)
		return (0);
	if (get_time_us() < dongle->ready_at)
		return (0);
	return (1);
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

int	dongle_take(t_coder *coder, t_dongle *dongle)
{
	t_scheduler	scheduler;

	scheduler = coder->sim->config.scheduler;
	add_waiter(coder, dongle);
	pthread_mutex_lock(&dongle->lock);
	while (!sim_stopped(coder->sim))
	{
		if (can_grant(coder, dongle))
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
