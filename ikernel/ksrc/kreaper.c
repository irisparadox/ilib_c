#include <kinclude/kfun.h>
#include <kinclude/kreaper.h>
#include <kinclude/sched.h>

struct kreaper *kernel_reaper(void)
{
	return &ikern_self()->reaper;
}

static void *kreaper_entry(void *arg)
{
	struct kreaper *r;
	struct itask *tsk;

	r = arg;

	do {
		/* Will return NULL if destruction.  */
		tsk = kreaper_dequeue(r);

		if (tsk == NULL)
			break;

		/* TODO free task resources */

	} while (1);

	return NULL;
}

void kreaper_init(struct kreaper *r, struct ikern *k)
{
	r->kernel = k;

	ilisti_init(&r->zombies);
	ilisti_init(&r->poison);
	ilisti_init(&r->children);

	pthread_mutex_init(&r->lock, NULL);
	pthread_cond_init(&r->grim_sig, NULL);

	if (pthread_create(&r->grim_thread, NULL, kreaper_entry, r) != 0) {
		pthread_mutex_destroy(&r->lock);
		pthread_cond_destroy(&r->grim_sig);

		BUG(); /* no reaper, no fun */
	}
}

void kreaper_exit(struct kreaper *r)
{
	/*
	 * We queue the poison pill
	 * This will make sure the reaper ends
	 * gracefully.
	 */
	kreaper_enqueue(r, NULL);

	pthread_join(r->grim_thread, NULL);

	pthread_mutex_destroy(&r->lock);
	pthread_cond_destroy(&r->grim_sig);
}

void kreaper_enqueue(struct kreaper *r, struct itask *tsk)
{
	struct ilinode *node;

	if (tsk != NULL)
		node = &tsk->sibling;
	else
		node = &r->poison;

	pthread_mutex_lock(&r->lock);

	/* Enqueue the task.  */
	ilisti_push_back(&r->zombies, node);
	pthread_cond_signal(&r->grim_sig);

	pthread_mutex_unlock(&r->lock);
}

struct itask *kreaper_dequeue(struct kreaper *r)
{
	pthread_mutex_lock(&r->lock);

	while (ilisti_empty(&r->zombies))
		pthread_cond_wait(&r->grim_sig, &r->lock);

	struct ilinode *zomb = ilisti_pop_front(&r->zombies);

	pthread_mutex_unlock(&r->lock);

	if (zomb == &r->poison) return NULL;

	struct itask *task = ILISTI_ENTRY(zomb, struct itask, sibling);
	return task;
}

void kreaper_adopt(struct kreaper *r, struct itask *tsk)
{
	ilisti_remove(&tsk->sibling);
	tsk->parent = NULL;
	ilisti_push_back(&r->children, &tsk->sibling);
}
