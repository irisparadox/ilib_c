#ifndef IDSCHED_H_
#define IDSCHED_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif  /* _POSIX_C_SOURCE */

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L
#error "idsched.h needs a POSIX-compliant (POSIX.1-2008) system (pthreads, unistd.h)"
#endif /* !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L */

#include <deftypei.h>
#include <ihstmap.h>
#include <icontext.h>
#include <ilisti.h>
#include <compiler_iattr.h>
#include <x86-64/cache.h>
#include <ifutex.h>
#include <ilinux/funfunc.h>

#define IDSCHED_CORE_OFFLINE  (1u << 0)
#define IDSCHED_CORE_ONLINE   (1u << 1)
#define IDSCHED_CORE_STOPPING (1u << 2)
#define IDSCHED_CORE_IDLE     (1u << 3)

#define IDSCHED_TASK_NEW      (1u << 0)
#define IDSCHED_TASK_RUNNING  (1u << 2)
#define IDSCHED_TASK_DONE     (1u << 3)
#define IDSCHED_TASK_BLOCKED  (1u << 4)


#define ISC_TASK_ON_RQ_NONE      0
#define ISC_TASK_ON_RQ_QUEUED    1
#define ISC_TASK_ON_RQ_MIGRATING 2

#define IDSCHED_ACPI_SIGARM   (1u << 0)
#define IDSCHED_ACPI_SIGFRD   (1u << 1)
#define IDSCHED_ACPI_SIGERR   (1u << 2)

#define IDSCHED_ALL_CORES     ((ilib_size_t)-1)

#ifndef IDSCHED_PRED_MIN_HISTORY
#define IDSCHED_PRED_MIN_HISTORY 16
#endif  /* IDSCHED_PRED_MIN_HISTORY */

#ifndef IDSCHED_DEFAULT_RUNTIME
#define IDSCHED_DEFAULT_RUNTIME 1000000ULL
#endif  /* IDSCHED_DEFAULT_RUNTIME */

#ifndef IDSCHED_EMA_ALPHA
#define IDSCHED_EMA_ALPHA 0.125f
#endif  /* IDSCHED_EMA_ALPHA */

#ifndef IDSCHED_STCK_SIZE
#define IDSCHED_STCK_SIZE (32 * 1024)
#endif  /* IDSCHED_STCK_SIZE */

#ifndef IDSCHED_DEBUG
#define IDSCHED_DEBUG 0
#endif  /* IDSCHED_DEBUG */

#if IDSCHED_DEBUG == 1
#include <stdio.h>
#endif

#ifndef IDSCHED_DIRTY_STACK_SANITIZE
#define IDSCHED_DIRTY_STACK_SANITIZE 0
#endif  /* IDSCHED_DIRTY_STACK_SANITIZE */

#define IDSCHED_WIFEXITED(status)   ((status) >= 0)
#define IDSCHED_WEXITSTATUS(status) (status)
#define IDSCHED_INVALID_TID ((ilib_uint64_t)-1)
#define IDSCHED_IDLE_TID ((ilib_uint64_t)-2)
#define IDSCHED_STOP_TID ((ilib_uint64_t)-3)

typedef ilib_byte_t   u8;
typedef ilib_uint64_t u64;
typedef ilib_int64_t  s64;

struct rqi;
struct rt_rqi;

struct idsched_class;

typedef struct idsched      idsched_t;
typedef struct idsched_acpi idsched_acpi_t;
typedef struct idsched_core idsched_core_t;
typedef struct idsched_task idsched_task_t;

typedef struct iwaitq_entry {
	ilinode_t       node;
	idsched_task_t *task;
	idsched_task_t *target;
} iwaitq_entry;

struct iwait_opts {
	idsched_task_t *wo_target;
	int             wo_stat;
	int             wo_flags;

	struct iwaitq_entry wait;
	int             notask_error;
};

typedef struct iwaitq {
	pthread_mutex_t lck;
	ilinode_t       head;
} iwaitq;

typedef struct ireaper {
	pthread_t       thread;

	pthread_mutex_t lck;
	pthread_cond_t  cv;

	ilinode_t       zombies;
	ilinode_t       children;

	idsched_t      *sched;

	ilinode_t       poison;
} ireaper;

typedef ilib_uint64_t idsched_tid;

struct idsched {
	ilib_size_t     ncores;
	ilib_size_t     online;

	ilib_uint64_t   nseq;
	idsched_core_t *cores;

	ireaper         reaper;

	pthread_mutex_t lck;
};

struct idsched_acpi {
	idsched_t    *sched;

	pthread_t     watcher;
	sigset_t      mask;

	ilib_uint32_t flags;
};

#define IDSCHED_SC_ARGC 6

struct idsched_task {
	int                       (*fn)(void *);
	void                       *arg;

	idsched_task_t             *prnt;

	ilinode_t                   children; /* list head.  */
	ilinode_t                   sibling;  /* node in parent's children list.  */

	iwaitq                      wait_chldexit;
	iwaitq_entry                wait;

	icontext_t                  ctx;
	icontext_t                  shctx;

	idsched_t                  *sched;
	idsched_core_t             *core;

	unsigned int                __state;

	u8                          on_rq;
	unsigned int                flags;
	idsched_tid                 tid;

	ilinode_t                   run_list;

	int                         exitst;

	u64                         exec_start;
	u64                         sum_exec_runtime;
	int                         prio;

	const struct idsched_class *sched_class;

	u64                         nivcsw;
	u64                         nvcsw;

	pthread_mutex_t             lck;
	pthread_cond_t              cv;

	int                         sc_nr;
	ilib_uintptr_t              sc_arg[IDSCHED_SC_ARGC];
	long                        sc_ret;
};

struct idsched_core {
	idsched_t       *sched;
	ilib_size_t      id;

	pthread_t        thread;

	pthread_mutex_t  lck;
	pthread_cond_t   cv;

        struct rqi      *rq;

	ilib_uint32_t    flags;
};


#define MAX_RT_PRIO 100
#define MAX_PRIO    140

#ifndef IDSCHED_SCHEDSTATS
#define IDSCHED_SCHEDSTATS 1
#endif  /* IDSCHED_SCHEDSTATS */

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
	unsigned int                nr_running;
	idsched_task_t             *curr;	/* execution ctx */
	idsched_task_t             *idle;

	/*
	 * Next cache line will hold the hot rq lock.
	 */
	u64                         nr_switches		__i__cacheline_aligned;

	pthread_mutex_t             __lock;

	struct rt_rqi               rt;

	idsched_task_t             *stop;
	const struct idsched_class *next_class;

#if IDSCHED_SCHEDSTATS == 1
	unsigned int yld_count;

	unsigned int sched_count;
	unsigned int sched_goidle;
#endif /* IDSCHED_SCHEDSTATS == 1 */
};

#define ISC_SM_IDLE    (-1)
#define ISC_SM_NONE    0
#define ISC_SM_PREEMPT 1

struct idsched_class {
	void (*enqueue_task) (struct rqi *rq, idsched_task_t *p, int flags);
	bool (*dequeue_task) (struct rqi *rq, idsched_task_t *p, int flags);

	void (*yield_task)   (struct rqi *rq);

	idsched_task_t *(*pick_task)(struct rqi *rq);

	void (*put_prev_task)(struct rqi *rq, idsched_task_t *p, idsched_task_t *next);
	void (*set_next_task)(struct rqi *rq, idsched_task_t *p, bool first);

	void (*task_tick)(struct rqi *rq, idsched_task_t *p);

	void (*update_curr)(struct rqi *rq);
};

#define DEFINE_IDSCHED_CLASS(name)                   	\
const struct idsched_class name##_sched_class		\
	__ialigned(__alignof__(struct idsched_class))

extern const struct idsched_class stop_sched_class;
extern const struct idsched_class rt_sched_class;
extern const struct idsched_class idle_sched_class;

static const struct idsched_class *const sched_classes[] = {
	&stop_sched_class,
	&rt_sched_class,
	&idle_sched_class,
};

#define IDSCHED_INVALID_CHILD ((idsched_task_t *)-1)

#define IDSCHED_SYS_NONE  0
#define IDSCHED_SYS_FORK  1
#define IDSCHED_SYS_YIELD 2
#define IDSCHED_SYS_WAIT  3
#define IDSCHED_SYS_EXIT  4
#define IDSCHED_SYS_EXEC  5
#define IDSCHED_SYS_WAIT4 6

#define ISC_WECHILD -1
#define ISC_WNOWAIT  0
#define ISC_WSUCCSS  1
#define ISC_WREAPED  2

#define CPUEBUSY  1
#define CPUEAGAIN 2
#define CPUEAVAIL 3

#define CPUSHUTOK 0

#define IDSCHED_DISPATCH_CONTINUE 0
#define IDSCHED_DISPATCH_STOP     1

int idsched_create(idsched_t *sched, ilib_size_t ncores);
int idsched_destroy(idsched_t *sched);

int idsched_core_startup(idsched_t *sched, ilib_size_t n);
int idsched_core_shutdown(idsched_t *sched, ilib_size_t n);

idsched_tid     idsched_task_create(idsched_t *sch, idsched_task_t *t, int (*fn)(void *), void *arg);
int             idsched_task_destroy(idsched_task_t *t);
int             idsched_task_submit(idsched_t *sch, idsched_task_t *t);
int             idsched_task_wait(int *wstatus);
int             idsched_task_waittask(idsched_task_t *t, int *wstatus);
int             idsched_task_yield(void);
int             idsched_task_exec(int (*fn)(void *), void *arg);
idsched_task_t *idsched_task_fork(void);

int idsched_run(idsched_t *sched);

#endif /* IDSCHED_H_ */

//#define IDSCHED_IMPLEMENTATION
#ifdef IDSCHED_IMPLEMENTATION
#ifndef I_IDSCH_IMPL
#define I_IDSCH_IMPL

#include <stdlib.h>
#include <time.h>

#ifndef IHSTMAP_IMPLEMENTATION
#define IHSTMAP_IMPLEMENTATION
#endif  /* IHSTMAP_IMPLEMENTATION */

#include <ihstmap.h>

#ifndef ILISTI_IMPLEMENTATION
#define ILISTI_IMPLEMENTATION
#endif  /* ILISTI_IMPLEMENTATION */

#include <ilisti.h>

#define I_ONEMALPHA (1.0f - IDSCHED_EMA_ALPHA)

#define _I__TASK_HAS(t, f) (((t)->__state & (f)) != 0)
#define _I__TASK_SET(t, f) ((t)->__state |= (f))
#define _I__TASK_CLR(t, f) ((t)->__state &= ~(f))
#define _I__TASK_TGL(t, f) ((t)->__state ^= (f))

#define _I__TASK_MSTAT                                                  \
	(IDSCHED_TASK_NEW | IDSCHED_TASK_RUNNING | \
	IDSCHED_TASK_BLOCKED | IDSCHED_TASK_DONE)

#define _I__TASK_STSTAT(t, s)                  \
	do {                                   \
		(t)->__state &= ~_I__TASK_MSTAT; \
		(t)->__state |= (s);             \
	} while (0)

#define _I__TIF_NEED_RESCHED (1u << 3)
#define _I__TIF____REAPED    (1u << 5)
#define _I__TIF____USROWN    (1u << 6)

#define i_tif_need_resched(t) \
	((t)->flags & _I__TIF_NEED_RESCHED)
#define i_tif_reaped(t) \
	((t)->flags & _I__TIF____REAPED)
#define i_tif_usrown(t) \
	((t)->flags & _I__TIF____USROWN)

#define schedstat_enabled()	iunlikely(IDSCHED_SCHEDSTATS)
#define schedstat_inc(var)	do { if (schedstat_enabled()) { var++; } } while (0)

static pthread_key_t i_core_key;

static inline idsched_core_t *i_core_self(void)
{
	return pthread_getspecific(i_core_key);
}

static inline struct rqi *cpu_rq(void)
{
	return i_core_self()->rq;
}

#define get_current() cpu_rq()->curr
#define current get_current()

static inline struct rqi *task_rq(idsched_task_t *p)
{
	return p->core->rq;
}

/* ---------------- PRIVATE METHODS ---------------- */

#if defined(__x86_64__)
static void i_fix_frame_chain(const stack_t *old_stack, const stack_t *new_stack, icontext_t *new_ctx)
{
	ilib_uintptr_t old_low;
	ilib_uintptr_t old_high;
	ilib_uintptr_t new_low;
	imreg_t       *rbp;
	imreg_t        saved;
	ilib_uintptr_t off;

	old_low  = (ilib_uintptr_t)old_stack->ss_sp;
	old_high = old_low + old_stack->ss_size;
	new_low  = (ilib_uintptr_t)new_stack->ss_sp;

	rbp = (imreg_t *)new_ctx->ic_mcontext.gregs[IMREG_RBP];

	while (1) {
		if ((ilib_uintptr_t)rbp < new_low ||
		    (ilib_uintptr_t)rbp >= new_low + new_stack->ss_size)
			break;

		saved = *rbp;

		if ((ilib_uintptr_t)saved < old_low ||
		    (ilib_uintptr_t)saved >= old_high)
			break;

		off  = (ilib_uintptr_t)saved - old_low;
		*rbp = (imreg_t)(new_low + off);

#if IDSCHED_DEBUG == 1
		printf("fix_frame_chain: rbp %p -> %p\n",
			(void *)saved,
			(void *)*rbp);
#endif

		rbp = (imreg_t *)*rbp;
	}
}

static void i_relocate_stack(const idsched_task_t *old_task, idsched_task_t *new_task,
			     const stack_t *old_stack, stack_t *new_stack)
{
	ilib_uintptr_t old_base;
	ilib_uintptr_t new_base;
	ilib_intptr_t delta;

	old_base = (ilib_uintptr_t)old_stack->ss_sp;
	new_base = (ilib_uintptr_t)new_stack->ss_sp;

	delta = (ilib_intptr_t)(new_base - old_base);

	ilib_uintptr_t *ptr = (ilib_uintptr_t *)new_base;

	ilib_size_t words =
		new_stack->ss_size / sizeof(ilib_uintptr_t);

	for (ilib_size_t i = 0; i < words; ++i, ++ptr) {
		ilib_uintptr_t val = *ptr;

		/*
		 * Pointer into parent's stack.
		 */
		if (val >= old_base &&
		    val < old_base + old_stack->ss_size) {
			*ptr = (ilib_uintptr_t)((ilib_intptr_t)val + delta);
			continue;
		}

		/*
		 * Fix task self references.
		 */
		if (val == (ilib_uintptr_t)old_task) {
			*ptr = (ilib_uintptr_t)new_task;
#if IDSCHED_DEBUG == 1
			printf("relocate_stack: fix task self references with val %p -> %p\n",
				(void *)val,
				(void *)new_task);
#endif
		}
	}
}
#endif /* defined(__x86_64__) */

/* SCHEDULER-SPACE & SYSCALLS */
static long i_sys_fork(idsched_task_t *t);
static long i_sys_exec(idsched_task_t *t);
static long i_sys_yield(idsched_task_t *t);
static long i_sys_wait(idsched_task_t *t);
static long i_sys_wait4(idsched_task_t *t);
static long i_sys_exit(idsched_task_t *t);

static void i_exit_notify(idsched_task_t *t);

static int             i_reaper_init(ireaper *r, idsched_t *sch);
static void            i_reaper_destroy(ireaper *r);
static void            i_reaper_enqueue(ireaper *r, idsched_task_t *t);
static idsched_task_t *i_reaper_dequeue(ireaper *r);
static void            i_reaper_adopt(ireaper *r, idsched_task_t *t);
static void           *i_reaper_main(void *arg);

static inline void i_set_cpu_online(idsched_core_t *cpu);

static inline void i_set_tsk_reaped(idsched_task_t *t);
static inline void i_set_tsk_usrown(idsched_task_t *t);

static inline int  i_need_resched(void);
static inline void i_clear_tsk_need_resched(idsched_task_t *t);
static inline void i_set_tsk_need_resched(idsched_task_t *t);
static void i_resched_curr(struct rqi *rq);
static void i_schedule(void);
static void i__schedule_loop(int sched_mode);
static void i__schedule(int sched_mode);
static void i_sched_entry(idsched_task_t *t);
static void i_task_entry(idsched_task_t *t);

static int  rq_is_running(struct rqi *rq);
static void i_ttwu_do_activate(struct rqi *rq, idsched_task_t *p, int flags);
static int  i_try_to_wake_up(idsched_task_t *p, unsigned state);
static void i_wake_up_new_task(idsched_task_t *p);

void enqueue_task(struct rqi *rq, idsched_task_t *p, int flags);
bool dequeue_task(struct rqi *rq, idsched_task_t *p, int flags);
void activate_task(struct rqi *rq, idsched_task_t *p, int flags);
void deactivate_task(struct rqi *rq, idsched_task_t *p, int flags);

static bool            dequeue_task_idle(struct rqi *rq, idsched_task_t *p, int flags);
static idsched_task_t *pick_task_idle(struct rqi *rq);
static void            put_prev_task_idle(struct rqi *rq, idsched_task_t *prev, idsched_task_t *next);
static void            set_next_task_idle(struct rqi *rq, idsched_task_t *next, bool first);
static void            task_tick_idle(struct rqi *rq, idsched_task_t *curr);
static void            update_curr_idle(struct rqi *rq);

static void            enqueue_task_stop(struct rqi *rq, idsched_task_t *p, int flags);
static bool            dequeue_task_stop(struct rqi *rq, idsched_task_t *p, int flags);
static void            yield_task_stop(struct rqi *rq);
static idsched_task_t *pick_task_stop(struct rqi *rq);
static void            put_prev_task_stop(struct rqi *rq, idsched_task_t *p, idsched_task_t *next);
static void            set_next_task_stop(struct rqi *rq, idsched_task_t *p, bool first);
static void            task_tick_stop(struct rqi *rq, idsched_task_t *p);
static void            update_curr_stop(struct rqi *rq);

static void            enqueue_task_rt(struct rqi *rq, idsched_task_t *p, int flags);
static bool            dequeue_task_rt(struct rqi *rq, idsched_task_t *p, int flags);
static void            yield_task_rt(struct rqi *rq);
static idsched_task_t *pick_task_rt(struct rqi *rq);
static void            put_prev_task_rt(struct rqi *rq, idsched_task_t *p, idsched_task_t *next);
static void            set_next_task_rt(struct rqi *rq, idsched_task_t *p, bool first);
static void            task_tick_rt(struct rqi *rq, idsched_task_t *p);
static void            update_curr_rt(struct rqi *rq);

static void cpurq_rt_prepare(struct rt_rqi *rt);
static struct rqi *cpurq_prepare(void);
static void cpu_submit_online(void);
static void cpurq_dispose(struct rqi *rq);

static inline bool cpu_is_online(idsched_core_t *cpu);
static inline bool cpu_is_offline(idsched_core_t *cpu);
static void cpu_stop_prepare(void);
static void cpu_idle_prepare(void);
static void i_do_idle(void);
static void idle_play_dead(void);
static int  cpu_down(idsched_core_t *cpu);
static void cpu_do_stopper(void);
static void cpurq_migrate(void);

static int i_dispatch_schedcall(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: SCHEDCALL dispatch %d for tid=%zu\n", t->sc_nr, t->tid);
#endif
	switch (t->sc_nr) {
	case IDSCHED_SYS_NONE:
		return IDSCHED_DISPATCH_CONTINUE;
	case IDSCHED_SYS_FORK:
		i_sys_fork(t);
		return IDSCHED_DISPATCH_CONTINUE;

	case IDSCHED_SYS_EXEC:
		i_sys_exec(t);
		return IDSCHED_DISPATCH_STOP;

	case IDSCHED_SYS_YIELD:
		i_sys_yield(t);
		return IDSCHED_DISPATCH_STOP;

	case IDSCHED_SYS_WAIT:
		i_sys_wait(t);

		if (_I__TASK_HAS(t, IDSCHED_TASK_BLOCKED))
			return IDSCHED_DISPATCH_STOP;

		return IDSCHED_DISPATCH_CONTINUE;

	case IDSCHED_SYS_WAIT4:
		i_sys_wait4(t);

		if (_I__TASK_HAS(t, IDSCHED_TASK_BLOCKED))
			return IDSCHED_DISPATCH_STOP;

		return IDSCHED_DISPATCH_CONTINUE;

	case IDSCHED_SYS_EXIT:
		i_sys_exit(t);
		return IDSCHED_DISPATCH_STOP;
	default:
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: SCHEDCALL Dispatch unknown nr %d", t->sc_nr);
#endif
		return IDSCHED_DISPATCH_STOP;
	}
}

static long i_schedcall(idsched_task_t *t, int nr)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: SCHEDCALL entry tid=%zu nr=%d\n", t->tid, nr);
	printf("[DEBUG_SCHED]: SCHEDCALL arg t=%p currt=%p\n",
		(void *)t,
	       (void *)i_core_self()->currt);
#endif
	t->sc_nr  = nr;

	iswapcontext(&t->ctx, &t->shctx);

	return t->sc_ret;
}

static void i_waitqadd(iwaitq *wq, iwaitq_entry *we)
{
	pthread_mutex_lock(&wq->lck);

	ilisti_push_back(&wq->head, &we->node);

	pthread_mutex_unlock(&wq->lck);
}

static void i_waitqremove(iwaitq *wq, iwaitq_entry *we)
{
	pthread_mutex_lock(&wq->lck);

	ilisti_remove(&we->node);

	pthread_mutex_unlock(&wq->lck);
}

static void i_waitqwakeone(iwaitq *wq, idsched_task_t *chld)
{
	iwaitq_entry *we;
	ilinode_t    *pos;
	ilinode_t    *tmp;

	pthread_mutex_lock(&wq->lck);

	ILISTI_FOREACH_SAFE(pos, tmp, &wq->head) {
		we = ILISTI_ENTRY(pos, iwaitq_entry, node);

		if (we->target != NULL && we->target != chld)
			continue;

		ilisti_remove(pos);

		pthread_mutex_unlock(&wq->lck);

		i_try_to_wake_up(we->task, IDSCHED_TASK_BLOCKED);
		return;
	}

	pthread_mutex_unlock(&wq->lck);
}

static int i_reaper_init(ireaper *r, idsched_t *sch)
{
	r->sched = sch;

	ilisti_init(&r->zombies);
	ilisti_init(&r->poison);
	ilisti_init(&r->children);

	pthread_mutex_init(&r->lck, NULL);
	pthread_cond_init(&r->cv, NULL);

	if (pthread_create(&r->thread, NULL, i_reaper_main, r) != 0) {
		pthread_mutex_destroy(&r->lck);
		pthread_cond_destroy(&r->cv);
		return -1;
	}

#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper created successfully\n");
	printf("[DEBUG_SCHED]: Reaper pointer %p\n", r);
#endif

	return 0;
}

static void i_reaper_destroy(ireaper *r)
{
	i_reaper_enqueue(r, NULL);

	pthread_join(r->thread, NULL);

	pthread_mutex_destroy(&r->lck);
	pthread_cond_destroy(&r->cv);

	r->sched = NULL;
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper destroyed successfully\n");
#endif
}

static void i_reaper_enqueue(ireaper *r, idsched_task_t *t)
{
	ilinode_t *node;

	/* The node we enqueue could be the poison pill to kill the
           reaper which would mean shutdown.  */
	if (t != NULL)
		node = &t->sibling;
	else
		node = &r->poison;

	pthread_mutex_lock(&r->lck);

	/* Enqueue the task.  */
	ilisti_push_back(&r->zombies, node);
	pthread_cond_signal(&r->cv); /* Wake the reaper, time to die.  */
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER enqueued and signaled\n");
	printf("[DEBUG_SCHED]: REAPER node is %s\n", t ? "Task" : "Poison Pill");
	printf("[DEBUG_SCHED]: REAPER node pointer %p\n", t);
#endif
	pthread_mutex_unlock(&r->lck);
}

static idsched_task_t *i_reaper_dequeue(ireaper *r)
{
	pthread_mutex_lock(&r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER dequeue in progress\n");
#endif
	while (ilisti_empty(&r->zombies))
		pthread_cond_wait(&r->cv, &r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER awoken to kill zombie\n");
#endif
	ilinode_t *zombn = ilisti_front(&r->zombies);
	ilisti_remove(zombn);

	pthread_mutex_unlock(&r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER zombie pointer %p\n", zombn);
#endif
	if (zombn == &r->poison) return NULL;

	idsched_task_t *t = ILISTI_ENTRY(zombn, idsched_task_t, sibling);
	return t;
}

static void i_reaper_adopt(ireaper *r, idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER adopting children %p\n", t);
#endif
	ilisti_remove(&t->sibling);
	t->prnt = NULL;
	ilisti_push_back(&r->children, &t->sibling);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER task %p was adopted!\n", t);
#endif
}

static void *i_reaper_main(void *arg)
{
	ireaper        *r;
	idsched_task_t *t;

	r = arg;
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER main loop start\n");
#endif
	for (;;) {
		t = i_reaper_dequeue(r); /* This will return NULL if destruction.  */

		/* Death can die too.  */
		if (t == NULL) break;

#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: REAPER killing %p\n", t);
#endif
		idsched_task_destroy(t);
		free(t);
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: REAPER killed task\n");
#endif
	}
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: REAPER exiting\n");
#endif
	return NULL;
}

/* Main loop */
static inline u64 clock_task(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (u64)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static inline int task_on_rq_queued(idsched_task_t *p)
{
	return IREAD_ONCE(u8, p->on_rq) == ISC_TASK_ON_RQ_QUEUED;
}

static inline bool sched_stop_runnable(struct rqi *rq)
{
	return rq->stop && task_on_rq_queued(rq->stop);
}

static inline bool sched_rt_runnable(struct rqi *rq)
{
	return rq->rt.rt_nr_running > 0;
}

static inline void add_nr_running(struct rqi *rq, unsigned count)
{
	rq->nr_running += count;
}

static inline void sub_nr_running(struct rqi *rq, unsigned count)
{
	rq->nr_running -= count;
}

static u64 update_curr_common(struct rqi *rq)
{
	idsched_task_t *curr = rq->curr;
	u64 now;
	s64 delta_exec;

	now = clock_task();

	delta_exec = (s64)(now - curr->exec_start);
	if (iunlikely(delta_exec <= 0))
		return delta_exec;

	curr->exec_start = now;
	curr->sum_exec_runtime += delta_exec;

	return delta_exec;
}

static inline int i_need_resched(void)
{
	return iunlikely(i_tif_need_resched(current));
}

static inline void i_clear_tsk_need_resched(idsched_task_t *t)
{
	atomic_landnot(t->flags, _I__TIF_NEED_RESCHED);
}

static inline void i_set_tsk_need_resched(idsched_task_t *t)
{
	atomic_lor(t->flags, _I__TIF_NEED_RESCHED);
}

static inline void i_set_tsk_reaped(idsched_task_t *t)
{
	atomic_lor(t->flags, _I__TIF____REAPED);
}

static inline void i_set_tsk_usrown(idsched_task_t *t)
{
	atomic_lor(t->flags, _I__TIF____USROWN);
}

static void i_resched_curr(struct rqi *rq)
{
	idsched_task_t *curr = rq->curr;

	if (i_tif_need_resched(curr))
		return;

	i_set_tsk_need_resched(curr);
}

static void put_prev_set_next_task(struct rqi *rq, idsched_task_t *prev, idsched_task_t *next)
{
	if (next == prev)
		return;

	prev->sched_class->put_prev_task(rq, prev, next);
	next->sched_class->set_next_task(rq, next, true);
}

static idsched_task_t *pick_next_task(struct rqi *rq)
{
	const struct idsched_class *class;
	idsched_task_t *p;
	u64 i;

	for (i = 0; i < IARRAY_SIZE(sched_classes); ++i) {
		class = sched_classes[i];

		p = class->pick_task(rq);

		if (ilikely(p)) {
			put_prev_set_next_task(rq, rq->curr, p);
			return p;
		}
	}

	/* idle class should always have runnable task.  */
	abort();
}

static int rq_is_running(struct rqi *rq)
{
	return rq->nr_running > 0;
}


/*
 * Enqueues p on run-queue, wake the core if it was idle.
 */
static void i_ttwu_do_activate(struct rqi *rq, idsched_task_t *p, int flags)
{
	bool was_idle = (rq->curr == rq->idle);

	activate_task(rq, p, flags);

	if (was_idle) {
		i_resched_curr(rq);
		futex_wake_one(&rq->curr->flags);
	}
}

static int i_try_to_wake_up(idsched_task_t *p, unsigned state)
{
	struct rqi *rq;

	pthread_mutex_lock(&p->lck);

	if (!(p->__state & state)) {
		pthread_mutex_unlock(&p->lck);
		return 0;
	}

	rq = task_rq(p);
	pthread_mutex_lock(&rq->__lock);

	if (p->on_rq) {
		pthread_mutex_unlock(&rq->__lock);
		pthread_mutex_unlock(&p->lck);
		return 0;
	}

	p->__state = IDSCHED_TASK_RUNNING;
	i_ttwu_do_activate(rq, p, 0);

	pthread_mutex_unlock(&rq->__lock);
	pthread_mutex_unlock(&p->lck);

	return 1;
}

static void i_wake_up_new_task(idsched_task_t *p)
{
	struct rqi *rq = task_rq(p);

	pthread_mutex_lock(&rq->__lock);
	i_ttwu_do_activate(rq, p, 0);
	pthread_mutex_unlock(&rq->__lock);
}

void enqueue_task(struct rqi *rq, idsched_task_t *p, int flags)
{
	p->sched_class->enqueue_task(rq, p, flags);
}

inline bool dequeue_task(struct rqi *rq, idsched_task_t *p, int flags)
{
	return p->sched_class->dequeue_task(rq, p, flags);
}

void activate_task(struct rqi *rq, idsched_task_t *p, int flags)
{
	/* TODO check on migration */

	enqueue_task(rq, p, flags);

	IWRITE_ONCE(u8, p->on_rq, ISC_TASK_ON_RQ_QUEUED);
}

void deactivate_task(struct rqi *rq, idsched_task_t *p, int flags)
{
	IWRITE_ONCE(u8, p->on_rq, ISC_TASK_ON_RQ_MIGRATING);
	dequeue_task(rq, p, flags);
}

/*
 * i__schedule() is the main scheduler function.
 *
 * This function is heavily based on Linux 7.2 kernel scheduler.
 * The scheduler is primarily driven through the following execution paths:
 *
 * 1. Explicit blocking: waitqueue.
 *
 *    A task may voluntarily block by waiting on a waitqueue. It is then
 *    that the control is yielded back to the scheduler. The task remains
 *    ineligible for execution until it is explicitly awakened.
 *
 * 2. Wakeups won't cause an entry into the scheduler. A task will be added
 *    to the run-queue and will wait to be scheduled the next cycle.
 *
 *    For now, there's no preemption mechanism built for this scheduler.
 *    This means tasks are executed from beginning to end, that is if the
 *    task doesn't yield.
 */
static void i__schedule(int sched_mode)
{
	idsched_task_t *prev, *next;
	char preempt = sched_mode > ISC_SM_NONE;
	char is_switch;
	unsigned long *switch_count;
	unsigned long prev_state;
	struct rqi *rq;

	rq = cpu_rq();
	prev = rq->curr;

	pthread_mutex_lock(&rq->__lock);

	switch_count = &prev->nivcsw;

	preempt = sched_mode == ISC_SM_PREEMPT;

	prev_state = IREAD_ONCE(unsigned int, prev->__state);
	if (sched_mode == ISC_SM_IDLE) {
		if (!rq->nr_running) {
			next = prev;
			rq->next_class = &idle_sched_class;
			goto picked;
		}
	} else if (!preempt &&
		   (prev_state & (IDSCHED_TASK_BLOCKED | IDSCHED_TASK_DONE))) {
		prev->sched_class->dequeue_task(rq, prev, 0);
		switch_count = &prev->nvcsw;
	}

	next = pick_next_task(rq);
	rq->next_class = next->sched_class;
picked:
	i_clear_tsk_need_resched(prev);
	is_switch = prev != next;
	if (ilikely(is_switch)) {
		rq->nr_switches++;
		++(*switch_count);

		_I__TASK_STSTAT(next, IDSCHED_TASK_RUNNING);
		rq->curr = next;

		pthread_mutex_unlock(&rq->__lock);

		iswapcontext(&prev->shctx, &next->shctx);
	} else {
		pthread_mutex_unlock(&rq->__lock);
	}
}

static inline void i__schedule_loop(int sched_mode)
{
	do {
		i__schedule(sched_mode);
	} while (i_need_resched());
}

static inline void i_schedule(void)
{
	i__schedule_loop(ISC_SM_NONE);
}

static inline void i_schedule_idle(void)
{
	do {
		i__schedule(ISC_SM_IDLE);
	} while (i_need_resched());
}


static void cpurq_rt_prepare(struct rt_rqi *rt)
{
	u64 i;

	for (i = 0; i < MAX_RT_PRIO; ++i) {
		ilisti_init(&rt->active.queue[i]);
	}

	memset(rt->active.bitmap, 0, sizeof(rt->active.bitmap));
	rt->rt_nr_running = 0;
	rt->rt_queued     = 0;
}

static struct rqi *cpurq_prepare(void)
{
	struct rqi *rq = calloc(1, sizeof(struct rqi));

	if (iunlikely(rq == NULL))
		return NULL;

	if (iunlikely(pthread_mutex_init(&rq->__lock, NULL) != 0)) {
		free(rq);
		return NULL;
	}

	cpurq_rt_prepare(&rq->rt);

	rq->nr_running  = 0;
	rq->nr_switches = 0;
	rq->curr        = NULL;
	rq->idle        = NULL;
	rq->stop        = NULL;
	rq->next_class  = &idle_sched_class;

	return rq;
}

static inline void i_set_cpu_online(idsched_core_t *cpu)
{
	cpu->flags = IDSCHED_CORE_ONLINE;
}

static inline bool cpu_is_online(idsched_core_t *cpu)
{
	return IREAD_ONCE(unsigned, cpu->flags) & IDSCHED_CORE_ONLINE;
}

static inline bool cpu_is_offline(idsched_core_t *cpu)
{
	return IREAD_ONCE(unsigned, cpu->flags) & IDSCHED_CORE_OFFLINE;
}

static void cpu_submit_online(void)
{
	idsched_core_t *cpu = i_core_self();
	pthread_mutex_lock(&cpu->lck);
	i_set_cpu_online(cpu);
	pthread_cond_signal(&cpu->cv);
	pthread_mutex_unlock(&cpu->lck);
}

static void cpurq_dispose(struct rqi *rq)
{
	if (iunlikely(rq == NULL))
		return;

	free(rq->stop->shctx.ic_stack.ss_sp);
	pthread_mutex_destroy(&rq->stop->lck);
	pthread_cond_destroy(&rq->stop->cv);
	free(rq->stop);

	pthread_mutex_destroy(&rq->idle->lck);
	pthread_cond_destroy(&rq->idle->cv);
	free(rq->idle);

	pthread_mutex_destroy(&rq->__lock);
	free(rq);
}


/*
 * This is the cpu entry.
 * It's where all the magic starts!
 */
static void i_core_run(void)
{
	cpu_idle_prepare();
	cpu_submit_online();
	while (1)
		i_do_idle();
}

/* Worker Main Logic */
static void *i_core_worker(void *arg)
{
	idsched_core_t *core;

	core = (idsched_core_t *)arg;
	pthread_setspecific(i_core_key, core);

	cpu_stop_prepare();
	i_core_run();

	return NULL;
}

static void i_sched_entry(idsched_task_t *t)
{
	for (;;) {
		iswapcontext(&t->shctx, &t->ctx);

		switch (i_dispatch_schedcall(t)) {
		case IDSCHED_DISPATCH_CONTINUE:
			t->sc_nr = IDSCHED_SYS_NONE;
			continue;
		case IDSCHED_DISPATCH_STOP:
		default:
			t->sc_nr = IDSCHED_SYS_NONE;
			i_schedule();
			break;
		}
	}
}

static void i_task_entry(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("i_task_entry: t=%p tid=%zu\n", (void *)t, t->tid);
#endif
	int status;

	status = t->fn(t->arg);

	t->sc_arg[0] = (ilib_uintptr_t)status;
	i_schedcall(t, IDSCHED_SYS_EXIT);
#if IDSCHED_DEBUG == 1
	/* Should never return */
	printf("ERROR: i_schedcall(EXIT) returned!\n");
#endif
	abort();
}

/* Checks for bootstrap */
static int i_is_bst_core(idsched_t *sched)
{
	return pthread_equal(pthread_self(), sched->cores[0].thread);
}

static idsched_core_t *i_core_shtdwn_n(idsched_t *sched)
{
	ilib_size_t i;

	for (i = 1; i < sched->ncores; ++i) {
		if ((sched->cores[i].flags & IDSCHED_CORE_ONLINE) &&
		    !(sched->cores[i].flags & IDSCHED_CORE_STOPPING))
			return &sched->cores[i];
	}

	return NULL;
}

static ilib_size_t i_cntr_flag(idsched_t *sched, ilib_uint32_t flag)
{
	ilib_size_t i;
	ilib_size_t cnt = 0;

	for (i = 1; i < sched->ncores; ++i) {
		if (sched->cores[i].flags & flag)
			++cnt;
	}

	return cnt;
}

/* SYSCALL IMPLEMENTATIONS */

/*
 * These two functions (wait_consider_task and do_wait_task)
 * are optimizations for waittask which follows the same idea
 * of waitpid.
 */
static long i_wait_consider_task(struct iwait_opts *wo)
{
	int retval;
	if (_I__TASK_HAS(wo->wo_target, IDSCHED_TASK_DONE)) {
		wo->wo_stat = wo->wo_target->exitst;

		ilisti_remove(&wo->wo_target->sibling);
		i_set_tsk_reaped(wo->wo_target);
		i_reaper_enqueue(&wo->wait.task->sched->reaper, wo->wo_target);

		retval = ISC_WREAPED;
		return retval;
	}

	retval = ISC_WNOWAIT;
	return retval;
}

static long i_do_wait_task(struct iwait_opts *wo)
{
	if (wo->wo_target == NULL || wo->wo_target->prnt != wo->wait.task)
		return ISC_WECHILD;

	return i_wait_consider_task(wo);
}

long i__do_wait(struct iwait_opts *wo)
{
	long retval;
	idsched_task_t *tsk;

	tsk = wo->wait.task;

	wo->notask_error = ISC_WECHILD;
	if (ilisti_empty(&tsk->children))
		goto notask;

	if (wo->wo_target != NULL) {
		retval = i_do_wait_task(wo);
		return retval;
	} else {
		ilinode_t *pos;
		ilinode_t *tmp;
		idsched_task_t *chld;

		ILISTI_FOREACH_SAFE(pos, tmp, &tsk->children) {
			chld = ILISTI_ENTRY(pos, idsched_task_t, sibling);

			if (!(_I__TASK_HAS(chld, IDSCHED_TASK_DONE)))
				continue;

			wo->wo_stat = chld->exitst;

			ilisti_remove(&chld->sibling);
			i_set_tsk_reaped(chld);
			i_reaper_enqueue(&tsk->sched->reaper, chld);

			tsk->sc_ret = 0;
			retval = ISC_WREAPED;
			return retval;
		}

		retval = ISC_WNOWAIT;
		return retval;
	}
 notask:
	retval = wo->notask_error;
	return retval;
}

static long i_do_wait(struct iwait_opts *wo)
{
	int retval;

	wo->wait.task = current;
	wo->wait.target = wo->wo_target;

	i_waitqadd(&current->wait_chldexit, &wo->wait);

	do {
		retval = i__do_wait(wo);

		if (ilikely(retval != ISC_WNOWAIT))
			break;

		_I__TASK_STSTAT(current, IDSCHED_TASK_BLOCKED);
		i_schedule();
	} while (1);

	i_waitqremove(&current->wait_chldexit, &wo->wait);

	_I__TASK_STSTAT(current, IDSCHED_TASK_RUNNING);

	return retval;
}

static long i_sys_wait(idsched_task_t *t)
{
	struct iwait_opts wo = {
		.wo_target = NULL,
		.wo_flags  = 0,
	};
	int *wstatus;
	int ret;

	wstatus = (int *)t->sc_arg[0];

	ret = i_do_wait(&wo);
	if (ret == ISC_WREAPED && wstatus != NULL)
		*wstatus = wo.wo_stat;

	t->sc_ret = (ret == ISC_WECHILD) ? -1 : 0;
	return ret;
}

static long i_sys_wait4(idsched_task_t *t)
{
	struct iwait_opts wo = {
		.wo_target = NULL,
		.wo_flags  = 0,
	};
	int *wstatus;
	long ret;

	wo.wait.task = NULL;

	wstatus = (int *)t->sc_arg[1];

	/* Resolve target from pid argument.  */
	wo.wo_target = (idsched_task_t *)t->sc_arg[0];

	ret = i_do_wait(&wo);

	if (ret == ISC_WREAPED && wstatus != NULL)
		*wstatus = wo.wo_stat;

	return ret;
}

/*
 * Fork routine.
 *
 * This function creates a new task and copies both the
 * kernel context (shctx) and user context (ctx). Because
 * this is a very rudimentary routine in user-space the stack
 * has to be relocated. This is because when we copy the stack,
 * the address space is still the same, whereas in real fork()
 * semantics, the copy-on-write aspect makes it so any references
 * in the stack are immediately valid thanks to virtual memory.
 *
 * The calls to relocation and frame fix try to "guess" any
 * references inside the parent's stack and fix them by relocating
 * them using the parent task as reference.
 *
 * If successful, it immediately marks the child ready for
 * execution and queues it.
 *
 * The parent's return value is set to the child's pointer
 * and the child's return value is set to NULL, following
 * fork() semantics. If fail it returns a pointer set to
 * IDSCHED_INVALID_CHILD (which is -1).
 *
 * WARNING: current implementation only works for x86-64. Other
 * architectures are NOT supported yet.
 */
static long i_sys_fork(idsched_task_t *t)
{
	idsched_task_t *prnt = t;
	idsched_core_t *core = t->core;
	idsched_task_t *chld = calloc(1, sizeof(idsched_task_t));
	if (iunlikely(chld == NULL)) {
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return (long)IDSCHED_INVALID_CHILD;
	}

	pthread_mutex_init(&chld->lck, NULL);
	pthread_cond_init(&chld->cv, NULL);

	pthread_mutex_lock(&core->sched->lck);
	chld->tid = core->sched->nseq++;
	pthread_mutex_unlock(&core->sched->lck);

	memcpy(&chld->ctx, &prnt->ctx, sizeof(icontext_t));
	memcpy(&chld->shctx, &prnt->shctx, sizeof(icontext_t));

	/* Technically not useful in practice since icontext ignores RAX.  */
	chld->ctx.ic_mcontext.gregs[IMREG_RDI] = (ilib_uintptr_t)chld;
	chld->ctx.ic_mcontext.gregs[IMREG_RAX] = 0;
#if IDSCHED_DEBUG == 1
	printf("fork copy:\n");
	printf("  parent RIP=%p RDI=%p RSP=%p\n",
		(void *)prnt->ctx.ic_mcontext.gregs[IMREG_RIP],
		(void *)prnt->ctx.ic_mcontext.gregs[IMREG_RDI],
		(void *)prnt->ctx.ic_mcontext.gregs[IMREG_RSP]);

	printf("  child  RIP=%p RDI=%p RSP=%p\n",
		(void *)chld->ctx.ic_mcontext.gregs[IMREG_RIP],
		(void *)chld->ctx.ic_mcontext.gregs[IMREG_RDI],
		(void *)chld->ctx.ic_mcontext.gregs[IMREG_RSP]);
#endif

	chld->ctx.ic_stack.ss_sp = malloc(prnt->ctx.ic_stack.ss_size);
	chld->ctx.ic_stack.ss_flags = 0;
	if (iunlikely(chld->ctx.ic_stack.ss_sp == NULL)) {
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld);
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return (long)IDSCHED_INVALID_CHILD;
	}

	chld->shctx.ic_stack.ss_sp = malloc(prnt->shctx.ic_stack.ss_size);
	chld->shctx.ic_stack.ss_flags = 0;
	if (iunlikely(chld->shctx.ic_stack.ss_sp == NULL)) {
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld->ctx.ic_stack.ss_sp);
		free(chld);
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return (long)IDSCHED_INVALID_CHILD;
	}

	chld->ctx.ic_stack.ss_size = prnt->ctx.ic_stack.ss_size;
	chld->shctx.ic_stack.ss_size = prnt->shctx.ic_stack.ss_size;

	memcpy(chld->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_size);

	memcpy(chld->shctx.ic_stack.ss_sp,
	       prnt->shctx.ic_stack.ss_sp,
	       prnt->shctx.ic_stack.ss_size);

	/* Okay. This was a dirty and provisional fix,
           but we leave it alone.  */
#if IDSCHED_DIRTY_STACK_SANITIZE == 1
	ilib_uintptr_t old_t = (ilib_uintptr_t)prnt;
	ilib_uintptr_t new_t = (ilib_uintptr_t)chld;

	ilib_uintptr_t *stack = (ilib_uintptr_t *)chld->ctx.ic_stack.ss_sp;
	ilib_size_t words = chld->ctx.ic_stack.ss_size / sizeof(ilib_uintptr_t);

	for (ilib_size_t i = 0; i < words; i++) {
		if (stack[i] == old_t)
			stack[i] = new_t;
	}
#else
	i_relocate_stack(prnt, chld, &prnt->ctx.ic_stack, &chld->ctx.ic_stack);
	i_relocate_stack(prnt, chld, &prnt->shctx.ic_stack, &chld->shctx.ic_stack);
	i_fix_frame_chain(&prnt->ctx.ic_stack, &chld->ctx.ic_stack, &chld->ctx);
	i_fix_frame_chain(&prnt->shctx.ic_stack, &chld->shctx.ic_stack, &chld->shctx);
#endif /* IDSCHED_DIRTY_STACK_SANITIZE == 1 */
#if IDSCHED_DEBUG == 1
	printf("stack copy parent t=%p child t=%p\n",
		(void *)prnt,
		(void *)chld);
#endif
	ilib_uintptr_t off = prnt->ctx.ic_mcontext.gregs[IMREG_RSP] -
			     (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;
	chld->ctx.ic_mcontext.gregs[IMREG_RSP] = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp + off;

	off = prnt->ctx.ic_mcontext.gregs[IMREG_RBP] - (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;
	chld->ctx.ic_mcontext.gregs[IMREG_RBP] = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp + off;

	off = prnt->shctx.ic_mcontext.gregs[IMREG_RSP] - (ilib_uintptr_t)prnt->shctx.ic_stack.ss_sp;
	chld->shctx.ic_mcontext.gregs[IMREG_RSP] = (ilib_uintptr_t)chld->shctx.ic_stack.ss_sp + off;

	off = prnt->shctx.ic_mcontext.gregs[IMREG_RBP] - (ilib_uintptr_t)prnt->shctx.ic_stack.ss_sp;
	chld->shctx.ic_mcontext.gregs[IMREG_RBP] = (ilib_uintptr_t)chld->shctx.ic_stack.ss_sp + off;

	chld->fn     = prnt->fn;
	chld->arg    = prnt->arg;

	ilisti_init(&chld->children);
	ilisti_init(&chld->sibling);

	chld->wait.task = chld;
	ilisti_init(&chld->wait.node);

	pthread_mutex_init(&chld->wait_chldexit.lck, NULL);
	ilisti_init(&chld->wait_chldexit.head);

	chld->core   = prnt->core;
	chld->prio   = prnt->prio;
	chld->nivcsw = prnt->nivcsw;
	chld->nvcsw  = prnt->nvcsw;
	chld->sched  = prnt->sched;
	chld->exitst = 0;
	chld->ctx.ic_link = &chld->shctx;

	chld->sched_class = prnt->sched_class;
	chld->on_rq = ISC_TASK_ON_RQ_NONE;

	chld->exec_start = 0;
	chld->sum_exec_runtime = 0;

	ilisti_init(&chld->run_list);
	chld->flags = 0;
	chld->__state = 0;
	_I__TASK_STSTAT(chld, IDSCHED_TASK_NEW);

	chld->sc_nr  = IDSCHED_SYS_NONE;

	prnt->sc_ret = (long)chld;
	chld->sc_ret = 0;

	chld->prnt   = prnt;
	ilisti_push_back(&prnt->children, &chld->sibling);

#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: FORK enqueue tid=%zu task=%p core=%p\n", chld->tid, (void *)chld, (void *)chld->core);
#endif

	i_wake_up_new_task(chld);
	return (long)chld;
}

static long i_sys_exec(idsched_task_t *t)
{
	int (*fn)(void *) = (int (*)(void *))t->sc_arg[0];
	void *arg = (void *)t->sc_arg[1];

	t->fn  = fn;
	t->arg = arg;

	imakecontext(&t->ctx, (void (*)(void))i_task_entry, 1, t);
	isetcontext(&t->ctx);

	/* unreachable */
	abort();
}

static long i_sys_yield(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: YIELD tid=%zu task=%p core=%p\n", t->tid, (void *)t, (void *)t->core);
#endif
	struct rqi *rq = task_rq(t);
	t->sched_class->yield_task(rq);

	t->sc_ret = 0;
	return 0;
}

static void i_exit_reparent(idsched_task_t *prnt)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: EXIT reparenting\n");
#endif
	ilinode_t      *pos;
	ilinode_t      *tmp;
	idsched_task_t *chld;

	ILISTI_FOREACH_SAFE(pos, tmp, &prnt->children) {
		chld = ILISTI_ENTRY(pos, idsched_task_t, sibling);

		i_reaper_adopt(&prnt->sched->reaper, chld);
	}
}

static void i_exit_notify(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: EXIT notify\n");
#endif
	_I__TASK_STSTAT(t, IDSCHED_TASK_DONE);
	i_exit_reparent(t);

	pthread_mutex_lock(&t->lck);
	pthread_cond_broadcast(&t->cv);
	pthread_mutex_unlock(&t->lck);

	if (t->prnt != NULL)
		i_waitqwakeone(&t->prnt->wait_chldexit, t);
	else if (!i_tif_usrown(t)) {
		ilisti_remove(&t->sibling);
		i_reaper_enqueue(&t->sched->reaper, t);
	}
}

static long i_sys_exit(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: EXIT syscall tid=%zu\n", t->tid);
#endif
	int status;

	status = (int)t->sc_arg[0];

	t->exitst = status;
	i_exit_notify(t);
	t->sc_ret = 0;
	return status;
}

/* ---------------- CREATION / DESTRUCTION ---------------- */

int idsched_create(idsched_t *sched, ilib_size_t ncores)
{
	ilib_size_t i;

	if (sched == NULL || ncores == 0)
		return -1;

	pthread_mutex_init(&sched->lck, NULL);

	sched->ncores = ncores;
	sched->nseq   = 0;
	sched->online = 0;
	sched->cores  = calloc(ncores, sizeof(idsched_core_t));

	if (iunlikely(sched->cores == NULL)) return -1;

	for (i = 0; i < ncores; ++i) {
		idsched_core_t *core = &sched->cores[i];

		core->sched = sched;
		core->id    = i;
		core->flags = IDSCHED_CORE_OFFLINE;

		pthread_mutex_init(&core->lck, NULL);
		pthread_cond_init(&core->cv, NULL);

		core->rq = cpurq_prepare();
		if (iunlikely(core->rq == NULL)) {
			ilib_size_t j;

			pthread_mutex_destroy(&core->lck);
			pthread_cond_destroy(&core->cv);

			for (j = 0; j < i; ++j) {
				cpurq_dispose(sched->cores[j].rq);
				pthread_mutex_destroy(&sched->cores[j].lck);
				pthread_cond_destroy(&sched->cores[j].cv);
			}

			free(sched->cores);
			sched->cores = NULL;
			pthread_mutex_destroy(&sched->lck);
			return -1;
		}
	}

	sched->cores[0].flags  = IDSCHED_CORE_ONLINE;
	sched->cores[0].thread = pthread_self();

	pthread_key_create(&i_core_key, NULL);
	i_reaper_init(&sched->reaper, sched);

	return 0;
}

int idsched_destroy(idsched_t *sched)
{
	ilib_size_t i;

	if (sched == NULL) return -1;

	i_reaper_destroy(&sched->reaper);

	int s = idsched_core_shutdown(sched, IDSCHED_ALL_CORES);

	if (s < 0) return -1;
	if (i_cntr_flag(sched, IDSCHED_CORE_ONLINE) != 0) return -1;

	sched->cores[0].flags = IDSCHED_CORE_OFFLINE;

	for (i = 0; i < sched->ncores; ++i) {
		idsched_core_t *core = &sched->cores[i];

		cpurq_dispose(core->rq);
		core->rq = NULL;

		pthread_mutex_destroy(&core->lck);
		pthread_cond_destroy(&core->cv);
	}

	free(sched->cores);
	pthread_mutex_destroy(&sched->lck);
	sched->cores  = NULL;
	sched->ncores = 0;
	sched->online = 0;
	sched->nseq   = 0;

	pthread_key_delete(i_core_key);

	return 0;
}

/* CORE MANAGEMENT: STARTUP & SHUTDOWN */

int idsched_core_startup(idsched_t *sched, ilib_size_t n)
{
	idsched_core_t *core;
	ilib_size_t     i;
	ilib_size_t     offline;
	ilib_size_t     started;

	if (sched == NULL || !i_is_bst_core(sched)) return -1;
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: BOOTSTRAP requested core startup of %zu cores\n", n);
#endif
	pthread_mutex_lock(&sched->lck);

	offline = i_cntr_flag(sched, IDSCHED_CORE_OFFLINE);

	if (n == IDSCHED_ALL_CORES || n > offline)
		n = offline;

	started = 0;

	for (i = 1; i < sched->ncores && started < n; ++i) {
		core = &sched->cores[i];

		if (!(core->flags & IDSCHED_CORE_OFFLINE)) {
			continue;
		}

		if (pthread_create(&core->thread, NULL, i_core_worker, core) != 0) {
			continue;
		}
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: BOOTSTRAP awaken Worker Core [%zu]%p", i, core);
#endif

		pthread_mutex_lock(&core->lck);

		while (!(core->flags & IDSCHED_CORE_ONLINE))
			pthread_cond_wait(&core->cv, &core->lck);

		pthread_mutex_unlock(&core->lck);

		++sched->online;
		++started;
	}

	pthread_mutex_unlock(&sched->lck);

	return (int)started;
}

int idsched_core_shutdown(idsched_t *sched, ilib_size_t n)
{
	idsched_core_t *core;
	ilib_size_t     online;
	ilib_size_t     stopped;

	if (sched == NULL || !i_is_bst_core(sched)) return -1;
#if IDSCHED_DEBUG == 1
	if (n == IDSCHED_ALL_CORES)
		printf("[DEBUG_SCHED]: BOOTSTRAP requested shutdown of all cores\n");
	else
		printf("[DEBUG_SCHED]: BOOTSTRAP requested shutdown of %zu cores\n", n);
#endif
	pthread_mutex_lock(&sched->lck);
	online = sched->online;

	if (n == IDSCHED_ALL_CORES || n > online)
		n = online;
#if IDSCHED_DEBUG == 1
	if (online <= 0)
		printf("[DEBUG_SCHED]: BOOTSTRAP no online cores available\n");
	else
		printf("[DEBUG_SCHED]: BOOTSTRAP will shutdown %zu cores\n", n);
#endif

	pthread_mutex_unlock(&sched->lck);
	stopped = 0;

	while (stopped < n) {
		pthread_mutex_lock(&sched->lck);
		core = i_core_shtdwn_n(sched);
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: Shutdown to core %p\n", core);
#endif

		if (iunlikely(core == NULL)) {
			pthread_mutex_unlock(&sched->lck);
			break;
		}
		core->flags |= IDSCHED_CORE_STOPPING;
		pthread_mutex_unlock(&sched->lck);

		if (cpu_down(core) != CPUSHUTOK)
			continue;
#if IDSCHED_DEBUG == 1
		printf("        -   Worker Core %p\n", core);
		printf("        -   thread %zu waiting for join()\n", core->thread);
#endif
		pthread_join(core->thread, NULL);
#if IDSCHED_DEBUG == 1
		printf("        [X] Worker Core %p joined\n", core);
#endif
		pthread_mutex_lock(&sched->lck);
		--sched->online;
		pthread_mutex_unlock(&sched->lck);
#if IDSCHED_DEBUG == 1
		printf("        [X] Worker Core %p shutdown\n", core);
#endif
		++stopped;
	}

	return (int)stopped;
}

/* TASK MANAGEMENT */

idsched_tid idsched_task_create(idsched_t *sch, idsched_task_t *t, int (*fn)(void *), void *arg)
{
	if (sch == NULL || t == NULL || fn == NULL) return -1;

	t->fn       = fn;
	t->arg      = arg;
	t->prnt     = NULL;
	t->sched    = sch;
	t->core     = NULL;
	t->__state  = 0;
	_I__TASK_STSTAT(t, IDSCHED_TASK_NEW);
	i_set_tsk_usrown(t);
	t->exitst   = 0;
	t->prio     = MAX_RT_PRIO - 1;
	t->on_rq    = ISC_TASK_ON_RQ_NONE;
	t->exec_start = 0;
	t->sum_exec_runtime = 0;

	t->nivcsw   = 0;
	t->nvcsw    = 0;

	ilisti_init(&t->children);
	ilisti_init(&t->sibling);
	ilisti_init(&t->run_list);

	ilisti_init(&t->wait.node);
	t->wait.task = t;

	pthread_mutex_init(&t->wait_chldexit.lck, NULL);
	ilisti_init(&t->wait_chldexit.head);


	t->ctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->ctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->ctx.ic_stack.ss_flags = 0;

	if (iunlikely(t->ctx.ic_stack.ss_sp == NULL)) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		return -1;
	}

	t->shctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->shctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->shctx.ic_stack.ss_flags = 0;

	if (iunlikely(t->shctx.ic_stack.ss_sp == NULL)) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		return -1;
	}

	if (iunlikely(igetcontext(&t->ctx) != 0)) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		free(t->shctx.ic_stack.ss_sp);
		return -1;
	}

	if (iunlikely(igetcontext(&t->shctx) != 0)) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		free(t->shctx.ic_stack.ss_sp);
		return -1;
	}

	t->shctx.ic_link = NULL;

	t->sched_class = &rt_sched_class;

	t->sc_nr  = IDSCHED_SYS_NONE;
	t->sc_ret = 0;

	pthread_mutex_init(&t->lck, NULL);
	pthread_cond_init(&t->cv, NULL);

	pthread_mutex_lock(&sch->lck);
	t->tid = sch->nseq++;
	pthread_mutex_unlock(&sch->lck);

	return t->tid;
}

int idsched_task_destroy(idsched_task_t *t)
{
	if (t == NULL || IREAD_ONCE(u8, t->on_rq))
		return -1;

	pthread_mutex_lock(&t->lck);
	if (!(_I__TASK_HAS(t, IDSCHED_TASK_DONE))) {
		pthread_mutex_unlock(&t->lck);
		return -1;
	}

	if (!ilisti_empty(&t->children)) {
		pthread_mutex_unlock(&t->lck);
		return -1;
	}
	pthread_mutex_unlock(&t->lck);

	pthread_mutex_destroy(&t->lck);
	pthread_cond_destroy(&t->cv);
	pthread_mutex_destroy(&t->wait_chldexit.lck);

	t->fn     = NULL;
	t->arg    = NULL;
	t->sched  = NULL;
	t->core   = NULL;
	_I__TASK_STSTAT(t, IDSCHED_TASK_NEW);
	t->tid    = 0;
	t->exitst = 0;

	t->prio = MAX_RT_PRIO - 1;
	t->on_rq = ISC_TASK_ON_RQ_NONE;
	t->exec_start = 0;
	t->sum_exec_runtime = 0;

	ilisti_init(&t->run_list);
	ilisti_init(&t->children);
	ilisti_init(&t->sibling);
	ilisti_init(&t->wait.node);
	ilisti_init(&t->wait_chldexit.head);

	t->sched_class = NULL;
	t->prnt = NULL;

	free(t->ctx.ic_stack.ss_sp);
	t->ctx.ic_stack.ss_sp   = NULL;
	t->ctx.ic_stack.ss_size = 0;

	free(t->shctx.ic_stack.ss_sp);
	t->shctx.ic_stack.ss_sp   = NULL;
	t->shctx.ic_stack.ss_size = 0;

	return 0;
}

int idsched_task_submit(idsched_t *sch, idsched_task_t *t)
{
	idsched_core_t *core;
	ilib_size_t     i, target, seen;

	if (sch == NULL || t == NULL) return -1;
	if (!(_I__TASK_HAS(t, IDSCHED_TASK_NEW))) return -1;

	pthread_mutex_lock(&sch->lck);

	if (sch->online == 0) {
		pthread_mutex_unlock(&sch->lck);
		return -1;
	}

	target = t->tid % sch->online;
	core = NULL;
	seen = 0;

	for (i = 1; i < sch->ncores; ++i) {
		if ((sch->cores[i].flags & IDSCHED_CORE_ONLINE) &&
		    !(sch->cores[i].flags & IDSCHED_CORE_STOPPING)) {
			if (seen == target) {
				core = &sch->cores[i];
				break;
			}
			++seen;
		}
	}

	if (core == NULL) {
		pthread_mutex_unlock(&sch->lck);
		return -1;
	}

	t->core  = core;
	t->ctx.ic_link = &t->shctx;

	imakecontext(&t->ctx, (void (*)(void))i_task_entry, 1, t);
	imakecontext(&t->shctx, (void (*)(void))i_sched_entry, 1, t);

	i_wake_up_new_task(t);
	pthread_mutex_unlock(&sch->lck);
	return 0;
}

int idsched_task_wait(int *wstatus)
{
	idsched_task_t *tsk;
	idsched_core_t *core;

	core = i_core_self();
	if (core == NULL) return -1;

	tsk = current;
	if (tsk == NULL) return -1;

	tsk->sc_arg[0] = (ilib_uintptr_t)wstatus;

	return (int)i_schedcall(tsk, IDSCHED_SYS_WAIT);
}

int idsched_task_waittask(idsched_task_t *t, int *wstatus)
{
	idsched_core_t *core;
	idsched_task_t *curr;
	struct rqi *rq;

	if (t == NULL)
		return -1;

	core = i_core_self();
	if (core != NULL) {
		rq = cpu_rq();
		curr = rq->curr;

		if (curr != rq->idle) {
			curr->sc_arg[0] = (ilib_uintptr_t)t;
			curr->sc_arg[1] = (ilib_uintptr_t)wstatus;
			curr->sc_arg[2] = 0;

			return (int)i_schedcall(curr, IDSCHED_SYS_WAIT4);
		}
	}

	pthread_mutex_lock(&t->lck);

	while (!_I__TASK_HAS(t, IDSCHED_TASK_DONE))
		pthread_cond_wait(&t->cv, &t->lck);

	if (wstatus != NULL)
		*wstatus = t->exitst;

	pthread_mutex_unlock(&t->lck);

	return 0;
}


int idsched_task_yield(void)
{
	idsched_core_t *core;

	core = i_core_self();
	if (core == NULL) return -1;

	if (current == NULL) return -1;

	return (int)i_schedcall(current, IDSCHED_SYS_YIELD);
}

int idsched_task_exec(int (*fn)(void *), void *arg)
{
	idsched_core_t *core = i_core_self();
	if (core == NULL) return -1;

	idsched_task_t *t = current;
	if (t == NULL) return -1;

	t->sc_arg[0] = (ilib_uintptr_t)fn;
	t->sc_arg[1] = (ilib_uintptr_t)arg;

	i_schedcall(t, IDSCHED_SYS_EXEC);

	/* should never return */
	return -1;
}

idsched_task_t *idsched_task_fork(void)
{
	idsched_task_t *prnt = current;

	return (idsched_task_t *)i_schedcall(prnt, IDSCHED_SYS_FORK);
}

int idsched_run(idsched_t *sched)
{
	if (sched == NULL) return -1;
	pthread_setspecific(i_core_key, &sched->cores[0]);

	i_core_run();

	return 0;
}

/* SCHEDULER CLASSES */

/* idle class */
static void cpu_idle_prepare(void)
{
	struct rqi *rq = cpu_rq();
	idsched_core_t *core = i_core_self();

	rq->idle = calloc(1, sizeof(idsched_task_t));
	if (iunlikely(rq->idle == NULL))
		BUG();
	/* we can't go on with initialization if idle can't be allocated */

	idsched_task_t *idle = rq->idle;
	idle->sched = core->sched;
	idle->core  = core;

	idle->tid   = IDSCHED_IDLE_TID;
	idle->prio  = MAX_PRIO - 1;

	idle->sched_class = &idle_sched_class;
	idle->flags = 0;

	ilisti_init(&idle->children);
	ilisti_init(&idle->sibling);
	ilisti_init(&idle->run_list);
	ilisti_init(&idle->wait.node);
	ilisti_init(&idle->wait_chldexit.head);

	pthread_mutex_init(&idle->lck, NULL);
	pthread_cond_init(&idle->cv, NULL);

	rq->curr    = idle;
	idle->on_rq = ISC_TASK_ON_RQ_QUEUED;
}

static void i_do_idle(void)
{
	idsched_core_t *cpu = i_core_self();
	struct rqi *rq = cpu_rq();

	if (cpu_is_offline(cpu)) {
		idle_play_dead();
	}

	while(!i_need_resched()) {
		futex_wait_zero(&rq->idle->flags);
	}

	i_schedule_idle();
}

static void idle_play_dead(void)
{
	pthread_exit(0);
}

static void put_prev_task_idle(struct rqi *rq, idsched_task_t *prev, idsched_task_t *next)
{
	(void)prev;
	(void)next;
	update_curr_idle(rq);
}

static void set_next_task_idle(struct rqi *rq, idsched_task_t *next, bool first)
{
	(void)next;
	(void)first;
	schedstat_inc(rq->sched_goidle);
}

idsched_task_t *pick_task_idle(struct rqi *rq)
{
	return rq->idle;
}

static bool dequeue_task_idle(struct rqi *rq, idsched_task_t *p, int flags)
{
	(void)rq;
	(void)p;
	(void)flags;
#if IDSCHED_DEBUG == 1
	printf("[ERR_SCHED]: Scheduling from idle task\n");
#endif
	return true;
}

static void task_tick_idle(struct rqi *rq, idsched_task_t *curr)
{
	(void)curr;
	update_curr_idle(rq);
}

static void update_curr_idle(struct rqi *rq)
{
	(void)rq;
}

DEFINE_IDSCHED_CLASS(idle) = {
	.dequeue_task		= dequeue_task_idle,

	.pick_task		= pick_task_idle,
	.put_prev_task		= put_prev_task_idle,
	.set_next_task		= set_next_task_idle,

	.task_tick		= task_tick_idle,

	.update_curr		= update_curr_idle,
};

/* stop class */
static void cpu_stop_prepare(void)
{
	struct rqi *rq = cpu_rq();
	idsched_core_t *cpu = i_core_self();

	rq->stop = calloc(1, sizeof(idsched_task_t));
	if (iunlikely(rq->stop == NULL))
		BUG();
	/* Stop needs to work, if not, this is fatal.  */

	idsched_task_t *stop = rq->stop;

	stop->shctx.ic_stack.ss_sp = malloc(IDSCHED_STCK_SIZE);
	if (iunlikely(stop->shctx.ic_stack.ss_sp == NULL)) {
		free(stop);
		BUG(); /* again, not cool...  */
	}

	stop->shctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	stop->shctx.ic_stack.ss_flags = 0;
	stop->shctx.ic_link = NULL;

	/* very unlikely to fail */
	if (iunlikely(igetcontext(&stop->shctx) != 0)) {
		free(stop->shctx.ic_stack.ss_sp);
		free(stop);
		BUG();
	}

	stop->fn   = NULL; /* We don't need this, just like idle.  */
	stop->arg  = NULL;
	stop->prnt = NULL;

	stop->sched = cpu->sched;
	stop->core  = cpu;

	/* technically calloc already does this,
           but let's just make it explicit.  */
	stop->__state = IDSCHED_TASK_BLOCKED;
	stop->on_rq   = ISC_TASK_ON_RQ_NONE;
	stop->flags   = 0;
	stop->tid     = IDSCHED_STOP_TID;
	stop->prio    = -1;

	stop->sched_class = &stop_sched_class;

	pthread_mutex_init(&stop->lck, NULL);
	pthread_cond_init(&stop->cv, NULL);

	/* Technically stop shouldn't have children ever but,
           we just make sure this is initialized :)  */
	ilisti_init(&stop->children);
	ilisti_init(&stop->sibling);
	ilisti_init(&stop->wait_chldexit.head);
	ilisti_init(&stop->wait.node);
	ilisti_init(&stop->run_list);

	imakecontext(&stop->shctx, cpu_do_stopper, 0);
}

static int cpu_down(idsched_core_t *cpu)
{
	struct rqi *rq = cpu->rq;

	if (rq_is_running(rq)) /* TODO we don't have migration yet so...  */
		return -CPUEBUSY;

	if (cpu->sched->online <= 0)
		return -CPUEAVAIL;

	int retval = i_try_to_wake_up(cpu->rq->stop, IDSCHED_TASK_BLOCKED);
	if (ilikely(retval)) {
		cpu->flags |= IDSCHED_CORE_STOPPING;
		return CPUSHUTOK;
	}

	return -CPUEAGAIN;
}

static void cpu_do_stopper(void)
{
	IWRITE_ONCE(unsigned, i_core_self()->flags, IDSCHED_CORE_OFFLINE);
	i_schedule();
}

static void cpurq_migrate(void)
{

}


static void set_next_task_stop(struct rqi *rq, idsched_task_t *p, bool first)
{
	(void)rq;
	(void)p;
	(void)first;
}

static idsched_task_t *pick_task_stop(struct rqi *rq)
{
	if (!sched_stop_runnable(rq))
		return NULL;

	return rq->stop;
}

static void enqueue_task_stop(struct rqi *rq, idsched_task_t *p, int flags)
{
	(void)flags;
	(void)p;
	add_nr_running(rq, 1);
}

static bool dequeue_task_stop(struct rqi *rq, idsched_task_t *p, int flags)
{
	(void)p;
	(void)flags;
	sub_nr_running(rq, 1);
	return true;
}

static void yield_task_stop(struct rqi *rq)
{
	(void)rq;
	BUG();
}

static void put_prev_task_stop(struct rqi *rq, idsched_task_t *p, idsched_task_t *next)
{
	(void)p;
	(void)next;
	update_curr_common(rq);
}

static void task_tick_stop(struct rqi *rq, idsched_task_t *p)
{
	(void)rq;
	(void)p;
}

static void update_curr_stop(struct rqi *rq)
{
	(void)rq;
}

DEFINE_IDSCHED_CLASS(stop) = {
	.enqueue_task		= enqueue_task_stop,
	.dequeue_task		= dequeue_task_stop,
	.yield_task		= yield_task_stop,

	.pick_task		= pick_task_stop,
	.put_prev_task		= put_prev_task_stop,
	.set_next_task		= set_next_task_stop,

	.task_tick		= task_tick_stop,

	.update_curr		= update_curr_stop,
};

static void requeue_task_rt(struct rqi *rq, idsched_task_t *p, int head)
{
	BUG_ON(p->on_rq != ISC_TASK_ON_RQ_QUEUED);
	ilisti_remove(&p->run_list);

	if (head)
		ilisti_push_front(&rq->rt.active.queue[p->prio], &p->run_list);
	else
		ilisti_push_back(&rq->rt.active.queue[p->prio], &p->run_list);
}

static void yield_task_rt(struct rqi *rq)
{
	requeue_task_rt(rq, rq->curr, 0);
}

static void update_curr_rt(struct rqi *rq)
{
	idsched_task_t *curr = rq->curr;
	s64 delta_exec;

	if (curr->sched_class != &rt_sched_class)
		return;

	delta_exec = update_curr_common(rq);
	if (iunlikely(delta_exec <= 0))
		return;
	/* TODO MAYBE ADD SOMETHING MORE HM?? */
}

static void enqueue_task_rt(struct rqi *rq, idsched_task_t *p, int flags)
{
	(void)flags;

	BUG_ON(p->on_rq != ISC_TASK_ON_RQ_NONE);
	BUG_ON(p->prio >= MAX_RT_PRIO);

	ilisti_push_back(&rq->rt.active.queue[p->prio], &p->run_list);

	bitmap_set(rq->rt.active.bitmap, p->prio);

	rq->rt.rt_nr_running++;
	add_nr_running(rq, 1);

	p->on_rq = ISC_TASK_ON_RQ_QUEUED;
}

static bool dequeue_task_rt(struct rqi *rq, idsched_task_t *p, int flags)
{
	(void)flags;
	update_curr_rt(rq);

	BUG_ON(p->on_rq != ISC_TASK_ON_RQ_QUEUED);

	ilisti_remove(&p->run_list);

	if (ilisti_empty(&rq->rt.active.queue[p->prio]))
		bitmap_clear(rq->rt.active.bitmap, p->prio);

	rq->rt.rt_nr_running--;
	sub_nr_running(rq, 1);

	p->on_rq = ISC_TASK_ON_RQ_NONE;

	return true;
}

static inline void set_next_task_rt(struct rqi *rq, idsched_task_t *p, bool first)
{
	(void)rq;
	(void)first;
	p->exec_start = clock_task();
}

static inline unsigned int rt_find_first_prio(const unsigned long *bitmap)
{
	unsigned int i;

	for (i = 0; i <= MAX_RT_PRIO; ++i) {
		if (bitmap_test(bitmap, i))
			return i;
	}

	return MAX_RT_PRIO;
}

static idsched_task_t *_pick_next_task_rt(struct rqi *rq)
{
	struct irt_prio_array *array = &rq->rt.active;
	ilinode_t *node;
	unsigned int prio;

	prio = rt_find_first_prio(array->bitmap);

	if (prio >= MAX_RT_PRIO)
		return NULL;

	node = ilisti_front(&array->queue[prio]);

	return ILISTI_ENTRY(node, idsched_task_t, run_list);
}

static idsched_task_t *pick_task_rt(struct rqi *rq)
{
	idsched_task_t *p;

	if (!sched_rt_runnable(rq))
		return NULL;

	p = _pick_next_task_rt(rq);

	return p;
}

static void put_prev_task_rt(struct rqi *rq, idsched_task_t *p, idsched_task_t *next)
{
	(void)next;
	(void)p;
	update_curr_rt(rq);
}

static void task_tick_rt(struct rqi *rq, idsched_task_t *p)
{
	(void)p;
	update_curr_rt(rq);
}

DEFINE_IDSCHED_CLASS(rt) = {
	.enqueue_task		= enqueue_task_rt,
	.dequeue_task		= dequeue_task_rt,
	.yield_task		= yield_task_rt,

	.pick_task		= pick_task_rt,
	.put_prev_task		= put_prev_task_rt,
	.set_next_task		= set_next_task_rt,

	.task_tick		= task_tick_rt,

	.update_curr		= update_curr_rt,
};

#endif /* I_IDSCH_IMPL */
#endif /* IDSCHED_IMPLEMENTATION */
