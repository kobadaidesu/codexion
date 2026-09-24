#include "../includes/codexion.h"

/*
** heap.c : dongle待ち行列(二分ヒープ)。呼び出し側がdongle->lock取得済み前提。
** FIFO: ticket昇順(そのdongleへの到着順)
** EDF : deadline昇順、同値ならcoder id昇順(tie-breaker)
*/

static int	higher(t_dongle *d, int a, int b, t_scheduler scheduler)
{
	if (scheduler == FIFO)
		return (d->wait_tickets[a] < d->wait_tickets[b]);
	if (d->wait_deadlines[a] != d->wait_deadlines[b])
		return (d->wait_deadlines[a] < d->wait_deadlines[b]);
	return (d->wait_ids[a] < d->wait_ids[b]);
}

static void	swap_slots(t_dongle *d, int a, int b)
{
	long long	tmp;

	tmp = d->wait_ids[a];
	d->wait_ids[a] = d->wait_ids[b];
	d->wait_ids[b] = (int)tmp;
	tmp = d->wait_tickets[a];
	d->wait_tickets[a] = d->wait_tickets[b];
	d->wait_tickets[b] = tmp;
	tmp = d->wait_deadlines[a];
	d->wait_deadlines[a] = d->wait_deadlines[b];
	d->wait_deadlines[b] = tmp;
}

void	heap_push(t_dongle *d, int coder_id,
	long long deadline, t_scheduler scheduler)
{
	int	index;
	int	parent;

	index = d->wait_count;
	d->wait_ids[index] = coder_id;
	d->wait_tickets[index] = d->next_ticket;
	d->next_ticket++;
	d->wait_deadlines[index] = deadline;
	d->wait_count++;
	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!higher(d, index, parent, scheduler))
			break ;
		swap_slots(d, index, parent);
		index = parent;
	}
}

int	heap_top_id(t_dongle *d)
{
	if (d->wait_count == 0)
		return (0);
	return (d->wait_ids[0]);
}

void	heap_pop(t_dongle *d, t_scheduler scheduler)
{
	int	index;
	int	child;

	if (d->wait_count == 0)
		return ;
	d->wait_count--;
	if (d->wait_count == 0)
		return ;
	d->wait_ids[0] = d->wait_ids[d->wait_count];
	d->wait_tickets[0] = d->wait_tickets[d->wait_count];
	d->wait_deadlines[0] = d->wait_deadlines[d->wait_count];
	index = 0;
	while (index * 2 + 1 < d->wait_count)
	{
		child = index * 2 + 1;
		if (child + 1 < d->wait_count
			&& higher(d, child + 1, child, scheduler))
			child++;
		if (!higher(d, child, index, scheduler))
			break ;
		swap_slots(d, index, child);
		index = child;
	}
}
