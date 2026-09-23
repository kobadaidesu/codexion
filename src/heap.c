#include "../includes/codexion.h"

/*
** heap.c : dongle待ち行列(二分ヒープ)の追加・取り出し・削除
** 呼び出し側がdongle->lockを取得していることが前提。
*/

static void	copy_slot(t_dongle *d, int dst, int src)
{
	d->wait_ids[dst] = d->wait_ids[src];
	d->wait_tickets[dst] = d->wait_tickets[src];
	d->wait_deadlines[dst] = d->wait_deadlines[src];
}

void	heap_push(t_dongle *dongle, int coder_id,
	long long deadline, t_scheduler scheduler)
{
	int	i;

	i = dongle->wait_count;
	dongle->wait_ids[i] = coder_id;
	dongle->wait_tickets[i] = dongle->next_ticket;
	dongle->next_ticket++;
	dongle->wait_deadlines[i] = deadline;
	dongle->wait_count++;
	heap_up(dongle, i, scheduler);
}

int	heap_top_id(t_dongle *dongle)
{
	if (dongle->wait_count == 0)
		return (0);
	return (dongle->wait_ids[0]);
}

void	heap_pop(t_dongle *dongle, t_scheduler scheduler)
{
	if (dongle->wait_count == 0)
		return ;
	dongle->wait_count--;
	if (dongle->wait_count == 0)
		return ;
	copy_slot(dongle, 0, dongle->wait_count);
	heap_down(dongle, 0, scheduler);
}

void	heap_remove(t_dongle *dongle, int coder_id, t_scheduler scheduler)
{
	int	i;

	i = 0;
	while (i < dongle->wait_count)
	{
		if (dongle->wait_ids[i] == coder_id)
		{
			dongle->wait_count--;
			if (i < dongle->wait_count)
			{
				copy_slot(dongle, i, dongle->wait_count);
				heap_fix(dongle, i, scheduler);
			}
			return ;
		}
		i++;
	}
}
