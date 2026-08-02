#ifndef KREAPER_H_
#define KREAPER_H_

#include <pthread.h>
#include <ilisti.h>

struct kreaper {
	pthread_t grim_thread;

	pthread_mutex_t lock;
	pthread_cond_t  grim_sig;

	struct ilinode zombies;
	struct ilinode children;
	struct ilinode poison;

	struct ikern *kernel;
};

extern struct kreaper *kernel_reaper(void);

extern void kreaper_init(struct kreaper *r, struct ikern *k);
extern void kreaper_exit(struct kreaper *r);

struct itask;

extern void kreaper_enqueue(struct kreaper *r, struct itask *tsk);
extern struct itask *kreaper_dequeue(struct kreaper *r);
extern void kreaper_adopt(struct kreaper *r, struct itask *tsk);

#endif // KREAPER_H_
