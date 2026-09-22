#include "../includes/codexion.h"

static long long	get_coder_deadline(t_coder *coder)
{
	long long	deadline;

	pthread_mutex_lock(&coder->sim->state_lock);
	deadline = coder->last_compile_start
		+ coder->sim->config.time_to_burnout * 1000;
	pthread_mutex_unlock(&coder->sim->state_lock);
	return (deadline);
}

static void	add_waiter(t_coder *coder, t_dongle *dongle)
{
	t_waiter	waiter;

	waiter.coder = coder;
	waiter.deadline = get_coder_deadline(coder);
	pthread_mutex_lock(&dongle->lock);
	waiter.ticket = dongle->next_ticket;
	dongle->next_ticket++;
	heap_push(&dongle->waiters, waiter, coder->sim->config.scheduler);
	pthread_mutex_unlock(&dongle->lock);
}

static int	can_grant(t_coder *coder, t_dongle *dongle)
{
	t_waiter	*top;

	top = heap_top(&dongle->waiters);
	if (top == NULL)
		return (0);
	if (top->coder != coder)
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
			heap_pop(&dongle->waiters, scheduler);
			dongle->holder = coder->id;
			pthread_mutex_unlock(&dongle->lock);
			return (1);
		}
		wait_dongle(dongle);
	}
	heap_remove_coder(&dongle->waiters, coder, scheduler);
	pthread_mutex_unlock(&dongle->lock);
	return (0);
}
