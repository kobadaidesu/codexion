#include "../includes/codexion.h"

/*
** dongle_release.c : 資源の解放とcooldown設定、待機threadの起床
** 使うのは dongle->lock のみ(state_lockとは入れ子にしない)。
*/

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

void	release_pair(t_coder *coder)
{
	dongle_release(coder, coder->left);
	dongle_release(coder, coder->right);
}

/* coders[i].right = dongles[i] の対応で全dongleを1回ずつ起こす */
void	wake_all_dongles(t_coder *coders, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_lock(&coders[i].right->lock);
		pthread_cond_broadcast(&coders[i].right->cond);
		pthread_mutex_unlock(&coders[i].right->lock);
		i++;
	}
}
