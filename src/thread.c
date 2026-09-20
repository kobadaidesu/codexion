#include "../includes/codexion.h"

/* STEP 4確認用の一時ルーチン:全員がわざとD1を取り合う */
static void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*dongle;

	coder = (t_coder *)arg;
	dongle = &coder->sim->dongles[0];
	while (!dongle_try_take(coder, dongle))
		usleep(1000);
	printf("coder %d took dongle %d\n", coder->id, dongle->id);
	usleep(10000);
	dongle_release(coder, dongle);
	return (NULL);
}

static int	join_threads(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}

t_error	run_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			join_threads(sim, i);
			return (ERR_THREAD);
		}
		i++;
	}
	if (join_threads(sim, i))
		return (ERR_THREAD);
	return (SUCCESS);
}
