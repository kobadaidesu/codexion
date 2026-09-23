#include "../includes/codexion.h"

/*
** heap_order.c : FIFO/EDF比較とヒープの並び直し
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

void	heap_up(t_dongle *dongle, int index, t_scheduler scheduler)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!higher(dongle, index, parent, scheduler))
			break ;
		swap_slots(dongle, index, parent);
		index = parent;
	}
}

void	heap_down(t_dongle *dongle, int index, t_scheduler scheduler)
{
	int	child;

	while (index * 2 + 1 < dongle->wait_count)
	{
		child = index * 2 + 1;
		if (child + 1 < dongle->wait_count
			&& higher(dongle, child + 1, child, scheduler))
			child++;
		if (!higher(dongle, child, index, scheduler))
			break ;
		swap_slots(dongle, index, child);
		index = child;
	}
}

void	heap_fix(t_dongle *dongle, int index, t_scheduler scheduler)
{
	int	parent;

	if (index <= 0)
	{
		heap_down(dongle, index, scheduler);
		return ;
	}
	parent = (index - 1) / 2;
	if (higher(dongle, index, parent, scheduler))
		heap_up(dongle, index, scheduler);
	else
		heap_down(dongle, index, scheduler);
}
