#ifndef SCHED_H_
#define SCHED_H_

#include <pthread.h>
#include <deftypei.h>
#include <ilisti.h>
#include <compiler_iattr.h>
#include <x86-64/cache.h>
#include <kinclude/kconf.h>
#include <kinclude/ktypes.h>

struct ikern;
struct ireaper;
struct ivcpu;
struct itask;

struct rqi;
struct rt_rqi;
struct isched_class;

struct ikern {

};

struct irt_prio_array {
	DECLARE_BITMAP(bitmap, MAX_RT_PRIO + 1); /* 1 bit for delimiter */
	ilinode_t queue[MAX_RT_PRIO];
};

struct rt_rqi {
	struct irt_prio_array active;
	unsigned int          rt_nr_running;
	int                   rt_queued;
};

struct rqi {
	unsigned int               	nr_running;
	struct itask               	*curr;	/* execution ctx */
	struct itask               	*idle;

	/*
	 * Next cache line will hold the hot rq lock.
	 */
	u64                        	nr_switches		__i__cacheline_aligned;

	pthread_mutex_t            	__lock;

	struct rt_rqi              	rt;

	struct itask               	*stop;
	const struct idsched_class 	*next_class;

#if KCONF_SCHED_STATS == 1
	unsigned int yld_count;

	unsigned int sched_count;
	unsigned int sched_goidle;
#endif /* IDSCHED_SCHEDSTATS == 1 */
};

struct isched_class {
	void (*enqueue_task) (struct rqi *rq, struct itask *p, int flags);
	bool (*dequeue_task) (struct rqi *rq, struct itask *p, int flags);

	void (*yield_task)   (struct rqi *rq);

	struct itask *(*pick_task)(struct rqi *rq);

	void (*put_prev_task)(struct rqi *rq, struct itask *p, struct itask *next);
	void (*set_next_task)(struct rqi *rq, struct itask *p, bool first);

	void (*task_tick)(struct rqi *rq, struct itask *p);

	void (*update_curr)(struct rqi *rq);
};

#define DEFINE_ISCHED_CLASS(name)                   	\
const struct isched_class name##_sched_class		\
	__ialigned(__alignof__(struct isched_class))

extern const struct isched_class stop_sched_class;
extern const struct isched_class rt_sched_class;
extern const struct isched_class idle_sched_class;

static const struct isched_class *const sched_classes[] = {
	&stop_sched_class,
	&rt_sched_class,
	&idle_sched_class,
};

#define schedstat_enabled()	unlikely(KCONF_SCHED_STATS)
#define schedstat_inc(var)	do { if (schedstat_enabled()) { var++; } } while(0)

#endif // SCHED_H_
