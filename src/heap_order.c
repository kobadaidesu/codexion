#include "../includes/codexion.h"

static int	higher_priority(t_waiter *a, t_waiter *b,
	t_scheduler scheduler)
{
	if (scheduler == FIFO)
		return (a->ticket < b->ticket);
	if (a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	return (a->coder->id < b->coder->id);
}

static void	swap_waiter(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_up(t_heap *heap, int index, t_scheduler scheduler)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!higher_priority(&heap->data[index],
				&heap->data[parent], scheduler))
			break ;
		swap_waiter(&heap->data[index], &heap->data[parent]);
		index = parent;
	}
}

void	heap_down(t_heap *heap, int index, t_scheduler scheduler)
{
	int	child;

	while (index * 2 + 1 < heap->size)
	{
		child = index * 2 + 1;
		if (child + 1 < heap->size
			&& higher_priority(&heap->data[child + 1],
				&heap->data[child], scheduler))
			child++;
		if (!higher_priority(&heap->data[child],
				&heap->data[index], scheduler))
			break ;
		swap_waiter(&heap->data[index], &heap->data[child]);
		index = child;
	}
}

void	heap_fix(t_heap *heap, int index, t_scheduler scheduler)
{
	int	parent;

	if (index <= 0)
	{
		heap_down(heap, index, scheduler);
		return ;
	}
	parent = (index - 1) / 2;
	if (higher_priority(&heap->data[index],
			&heap->data[parent], scheduler))
		heap_up(heap, index, scheduler);
	else
		heap_down(heap, index, scheduler);
}
