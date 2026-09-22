#include "../includes/codexion.h"

void	heap_push(t_heap *heap, t_waiter waiter, t_scheduler scheduler)
{
	heap->data[heap->size] = waiter;
	heap->size++;
	heap_up(heap, heap->size - 1, scheduler);
}

t_waiter	*heap_top(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (&heap->data[0]);
}

void	heap_pop(t_heap *heap, t_scheduler scheduler)
{
	if (heap->size == 0)
		return ;
	heap->size--;
	if (heap->size == 0)
		return ;
	heap->data[0] = heap->data[heap->size];
	heap_down(heap, 0, scheduler);
}

void	heap_remove_coder(t_heap *heap, t_coder *coder,
	t_scheduler scheduler)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->data[i].coder == coder)
		{
			heap->size--;
			if (i < heap->size)
			{
				heap->data[i] = heap->data[heap->size];
				heap_fix(heap, i, scheduler);
			}
			return ;
		}
		i++;
	}
}
