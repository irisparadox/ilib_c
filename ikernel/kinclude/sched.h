#ifndef SCHED_H_
#define SCHED_H_

#include <pthread.h>
#include <deftypei.h>
#include <ilisti.h>
#include <compiler_iattr.h>
#include <icontext.h>
#include <cache.h>
#include <ithread.h>
#include <kinclude/kconf.h>
#include <kinclude/ktypes.h>
#include <kinclude/vcpu.h>
#include <kinclude/wait.h>
#include <kinclude/ktid.h>
#include <kinclude/kreaper.h>

struct ikern;
struct ireaper;
struct ivcpu;
struct itask;
struct ithread_struct;

struct rqi;
struct rt_rqi;
struct isched_class;

#define SM_NONE	0
#define SM_IDLE	1
#define SM_PREEMPT	2

struct ikern {
	struct kreaper		reaper;
	struct ktid_alloc	tid_al;
	struct ivcpu_array	vcpus;

	pthread_rwlock_t	tasklist_lock;
};

#define SC_ARGC		6
#define TASK_COMM_LEN		16

#define TIF_NEED_RESCHED	(1u << 3)
#define TIF____REAPED		(1u << 5)

/* Used in tsk->__state: */
#define TASK_RUNNING		0x00000000
#define TASK_INTERRUPTIBLE	0x00000001
#define TASK_UNINTERRUPTIBLE	0x00000002
#define TASK_DEAD		0x00000080
#define TASK_NOLOAD		0x00000400
#define TASK_NEW		0x00000800
/* Used in tsk->exit_state: */
#define EXIT_DEAD		0x00000010
#define EXIT_ZOMBIE		0x00000020

#define TASK_IDLE		(TASK_UNINTERRUPTIBLE | TASK_NOLOAD)

struct itask {
	/*
	 * Must be the first element for easy access
	 * to thread.sp
	 */
	struct ithread_struct		thread;

	void				*stack;

	unsigned int			__state;
	u8				on_rq;

	/* Per task flags (TF_*): */
	unsigned int			flags;

	struct ktid			*tid;

	int				prio;
	unsigned int			rt_priority;

	const struct isched_class	*sched_class;
	struct ilinode			run_list;
	u64				exec_start;
	u64				sum_exec_runtime;

	int				exit_state;
	int				exit_code;

	/* Link to parent task, wait4() reports: */
	struct itask			*parent;

	/*
	 * Children/sibling form the list of natural children:
	 */
	struct ilinode			children;
	struct ilinode			sibling;

	struct iwait_queue_head	wait_chldexit;

	/* Context switch counters: */
	u64				nvcsw;
	u64				nivcsw;

	int				(*threadfn)(void *);
	void				*data;

	/*
	 * Synchronization primitives used by task sleep/wakeup paths.
	 */
	struct iwait_queue_entry	wait_entry;
	pthread_mutex_t		sleep_lock;
	pthread_cond_t			sleep_signal;

	char				comm[TASK_COMM_LEN];
};

#define MAX_PRIO	(MAX_RT_PRIO + NICE_WIDTH)
#define DEFAULT_PRIO	(MAX_RT_PRIO + NICE_WIDTH / 2)

struct irt_prio_array {
	DECLARE_BITMAP(bitmap, MAX_RT_PRIO + 1); /* 1 bit for delimiter */
	ilinode_t queue[MAX_RT_PRIO];
};

struct rt_rqi {
	struct irt_prio_array		active;
	unsigned int			rt_nr_running;
	int				rt_queued;
};

struct rqi {
	unsigned int			nr_running;
	struct itask			*curr;	/* execution ctx */
	struct itask			*idle;

	/*
	 * Next cache line will hold the hot rq lock.
	 */
	u64				nr_switches	__i__cacheline_aligned;

	pthread_mutex_t		__lock;

	struct rt_rqi			rt;

	struct itask			*stop;
	const struct isched_class	*next_class;

#if KCONF_SCHED_STATS == 1
	unsigned int yld_count;

	unsigned int sched_count;
	unsigned int sched_goidle;
#endif /* IDSCHED_SCHEDSTATS == 1 */
};

#define TASK_ON_RQ_NONE	0
#define TASK_ON_RQ_QUEUED	1
#define TASK_ON_RQ_MIGRATING	2

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

#define DEFINE_ISCHED_CLASS(name)			\
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

extern __thread void		*current_kernel_rsp;
extern __thread void		*saved_user_rsp;
extern __thread struct itask	*current_task;
#define current		current_task

#define tasklist_lock		ikern_self()->tasklist_lock
#define read_lock(lock)	pthread_rwlock_rdlock(lock)
#define read_unlock(lock)	pthread_rwlock_unlock(lock)
#define write_lock(lock)	pthread_rwlock_wrlock(lock)
#define write_unlock(lock)	pthread_rwlock_unlock(lock)

#define schedstat_enabled()	unlikely(KCONF_SCHED_STATS)
#define schedstat_inc(var)	do { if (schedstat_enabled()) { var++; } } while(0)

#define tif_test_bit(bit)	arch_test_bit(bit, (unsigned long *)&current->flags)
#define tif_need_resched()	tif_test_bit(TIF_NEED_RESCHED)

#define __set_current_state(state_value)					\
	do {									\
		IWRITE_ONCE(unsigned int, current->__state, (state_value));	\
	} while (0)

#define set_current_state(state_value)						\
	do {									\
		IWRITE_ONCE(unsigned int, current->__state, (state_value));	\
		__sync_synchronize();						\
	} while (0)

static inline void clear_tsk_need_resched(struct itask *tsk)
{
	__katomic_long_andnot(TIF_NEED_RESCHED, (__katomic_long_t *)&tsk->flags);
}

static __ialways_inline bool need_resched(void)
{
	return unlikely(tif_need_resched());
}

extern void schedule(void);
extern void schedule_idle(void);

#endif // SCHED_H_
