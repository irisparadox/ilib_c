#ifndef IDSCHED_H_
#define IDSCHED_H_

#include "x86-64/i_context.h"
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
#include <queue.h>
#include <ihstmap.h>
#include <icontext.h>
#include <ilisti.h>

#define IDSCHED_CORE_OFFLINE  (1u << 0)
#define IDSCHED_CORE_ONLINE   (1u << 1)
#define IDSCHED_CORE_STOPPING (1u << 2)
#define IDSCHED_CORE_IDLE     (1u << 3)

#define IDSCHED_TASK_NEW      (1u << 0)
#define IDSCHED_TASK_READY    (1u << 1)
#define IDSCHED_TASK_RUNNING  (1u << 2)
#define IDSCHED_TASK_DONE     (1u << 3)
#define IDSCHED_TASK_BLOCKED  (1u << 4)
#define IDSCHED_TASK_REAPED   (1u << 5)
#define IDSCHED_TASK_USROWN   (1u << 6)

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

typedef struct idsched      idsched_t;
typedef struct idsched_acpi idsched_acpi_t;
typedef struct idsched_core idsched_core_t;
typedef struct idsched_task idsched_task_t;

typedef struct i_pred_entry ipred_entry_t;
typedef struct i_pred       ipred_t;

struct i_pred {
	pthread_mutex_t lck;
	ihstmap_t       table;
};

struct i_pred_entry {
	ilib_uint64_t ema;
	ilib_uint64_t nsa;
};

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

	ipred_t         gpred;

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
	int  (*fn)(void *);
	void  *arg;

	idsched_task_t *prnt;

	ilinode_t children; /* list head.  */
	ilinode_t sibling;  /* node in parent's children list.  */

	iwaitq wait_chldexit;
	iwaitq_entry wait;

	icontext_t      ctx;
	icontext_t      shctx;

	idsched_t      *sched;
	idsched_core_t *core;

	ilib_uint32_t   flags;
	idsched_tid     tid;

	int exitst;

	ilib_uint64_t   rt;    /* runtime */
	ilib_uint64_t   pred;  /* prediction */
	ilib_uint64_t   prio;  /* priority */

	pthread_mutex_t lck;
	pthread_cond_t  cv;

	int   sc_nr;
	ilib_uintptr_t sc_arg[IDSCHED_SC_ARGC];
	long  sc_ret;
};

struct idsched_core {
	idsched_t      *sched;
	ilib_size_t     id;

	icontext_t      bootctx;

	pthread_t       thread;

	pthread_mutex_t lck;
	pthread_cond_t  cv;

	pqueue_t        rq;
	ipred_t         lpred;

	idsched_task_t *currt;
	idsched_task_t  idle;

	ilib_uint32_t   flags;
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

#define IDSCHED_IMPLEMENTATION
#ifdef IDSCHED_IMPLEMENTATION
#ifndef I_IDSCH_IMPL
#define I_IDSCH_IMPL

#include <stdlib.h>

#ifndef PQUEUE_IMPLEMENTATION
#define PQUEUE_IMPLEMENTATION
#endif  /* PQUEUE_IMPLEMENTATION */

#include "queue.h"

#include <time.h>

#ifndef IHSTMAP_IMPLEMENTATION
#define IHSTMAP_IMPLEMENTATION
#endif  /* IHSTMAP_IMPLEMENTATION */

#include "ihstmap.h"

#ifndef ILISTI_IMPLEMENTATION
#define ILISTI_IMPLEMENTATION
#endif  /* ILISTI_IMPLEMENTATION */

#include <ilisti.h>

#define I_ONEMALPHA (1.0f - IDSCHED_EMA_ALPHA)

#define _I__TASK_HAS(t, f) (((t)->flags & (f)) != 0)
#define _I__TASK_SET(t, f) ((t)->flags |= (f))
#define _I__TASK_CLR(t, f) ((t)->flags &= ~(f))
#define _I__TASK_TGL(t, f) ((t)->flags ^= (f))

#define _I__TASK_MSTAT                                                  \
	(IDSCHED_TASK_NEW | IDSCHED_TASK_READY | IDSCHED_TASK_RUNNING | \
	IDSCHED_TASK_BLOCKED | IDSCHED_TASK_DONE)

#define _I__TASK_STSTAT(t, s)                  \
	do {                                   \
		(t)->flags &= ~_I__TASK_MSTAT; \
		(t)->flags |= (s);             \
	} while (0)

static pthread_key_t i_core_key;

static idsched_core_t *i_core_self(void)
{
	return pthread_getspecific(i_core_key);
}

static int i_pred_update(ipred_t *p, int (*fn)(void *), ilib_uint64_t rt);

static int i_task_cmp(const void *a, const void *b)
{
	const idsched_task_t *ta;
	const idsched_task_t *tb;
	int                   pc;
	int                   idc;

	ta = *(idsched_task_t * const *)a;
	tb = *(idsched_task_t * const *)b;

	pc  = (ta->prio > tb->prio) -
	      (ta->prio < tb->prio);

	idc = (ta->tid > tb->tid) -
	      (ta->tid < tb->tid);

	return pc != 0 ? pc : idc;
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

static void i_schedule(void);
static void i__schedule_loop(idsched_core_t *core);
static void i__schedule(idsched_core_t *core, idsched_task_t *prev);
static void i_sched_entry(idsched_task_t *t);
static void i_task_entry(idsched_task_t *t);


static int idle_swapper_loop(void *arg)
{
	(void)arg;

	do {
		idsched_task_yield();
	} while (1);

	return 0;
}

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

		_I__TASK_STSTAT(we->task, IDSCHED_TASK_READY);

		pthread_mutex_lock(&we->task->core->lck);
		pqueue_push(&we->task->core->rq, &we->task);
		pthread_cond_signal(&we->task->core->cv);
		pthread_mutex_unlock(&we->task->core->lck);

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

static void i__schedule(idsched_core_t *core, idsched_task_t *prev)
{
	idsched_task_t *next;

	pthread_mutex_lock(&core->lck);
	pthread_mutex_lock(&prev->lck);

	if (prev != &core->idle && _I__TASK_HAS(prev, IDSCHED_TASK_READY))
		pqueue_push(&core->rq, &prev);

	pthread_mutex_unlock(&prev->lck);

	if (pqueue_empty(&core->rq))
		goto idle;

	next = pqueue_top(&core->rq, idsched_task_t *);
	pqueue_pop(&core->rq);
	goto picked;

idle:
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: __SCHEDULE idle branch\n");
#endif
	next = &core->idle;

	if (core->flags & IDSCHED_CORE_STOPPING) {
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: __SCHEDULE idle shutdown core\n");
#endif
		pthread_mutex_unlock(&core->lck);
		isetcontext(&core->bootctx);
		abort();
	}

picked:
	if (next == prev)
		goto unlock;

	core->currt = next;
	_I__TASK_STSTAT(next, IDSCHED_TASK_RUNNING);

	pthread_mutex_unlock(&core->lck);

	isetcontext(&next->shctx);

	abort();

unlock:
	pthread_mutex_unlock(&core->lck);
}

static void i__schedule_loop(idsched_core_t *core)
{
	do {
		i__schedule(core, core->currt);
	} while (0);
}

static inline void i_schedule(void)
{
	i__schedule_loop(i_core_self());
}

static void i_core_run(idsched_core_t *core)
{
	idsched_task_t *idle = &core->idle;
	memset(idle, 0, sizeof(idsched_task_t));

	idle->fn    = idle_swapper_loop;
	idle->arg   = NULL;
	idle->prnt  = NULL;
	idle->core  = core;
	idle->sched = core->sched;

	_I__TASK_STSTAT(idle, IDSCHED_TASK_RUNNING);

	idle->exitst = 0;
	idle->rt     = 0;
	idle->pred   = 0;
	idle->prio   = 0;

	ilisti_init(&idle->children);
	ilisti_init(&idle->sibling);

	ilisti_init(&idle->wait.node);
	idle->wait.task = idle;

	pthread_mutex_init(&idle->wait_chldexit.lck, NULL);
	ilisti_init(&idle->wait_chldexit.head);

	idle->ctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	idle->ctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	idle->ctx.ic_stack.ss_flags = 0;

	if (idle->ctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&idle->wait_chldexit.lck);
		return;
	}

	idle->shctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	idle->shctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	idle->shctx.ic_stack.ss_flags = 0;

	if (idle->shctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&idle->wait_chldexit.lck);
		free(idle->ctx.ic_stack.ss_sp);
		return;
	}

	if (igetcontext(&idle->ctx) != 0) {
		pthread_mutex_destroy(&idle->wait_chldexit.lck);
		free(idle->ctx.ic_stack.ss_sp);
		free(idle->shctx.ic_stack.ss_sp);
		return;
	}

	if (igetcontext(&idle->shctx) != 0) {
		pthread_mutex_destroy(&idle->wait_chldexit.lck);
		free(idle->ctx.ic_stack.ss_sp);
		free(idle->shctx.ic_stack.ss_sp);
		return;
	}

	idle->shctx.ic_link = NULL;

	idle->sc_nr  = IDSCHED_SYS_NONE;
	idle->sc_ret = 0;

	pthread_mutex_init(&idle->lck, NULL);
	pthread_cond_init(&idle->cv, NULL);

	idle->tid = IDSCHED_IDLE_TID;

	imakecontext(&idle->ctx, (void (*)(void))idle_swapper_loop, 1, idle);
	imakecontext(&idle->shctx, (void (*)(void))i_sched_entry, 1, idle);

	igetcontext(&core->bootctx);

	if (core->flags & IDSCHED_CORE_STOPPING) {
		pthread_mutex_destroy(&idle->wait_chldexit.lck);
		pthread_mutex_destroy(&idle->lck);
		pthread_cond_destroy(&idle->cv);
		free(idle->ctx.ic_stack.ss_sp);
		free(idle->shctx.ic_stack.ss_sp);
		return;
	}

	core->currt = idle;
	isetcontext(&idle->shctx);

	abort();
}

/* Worker Main Logic */
static void *i_core_worker(void *arg)
{
	idsched_core_t *core;

	core = (idsched_core_t *)arg;
	pthread_setspecific(i_core_key, core);

	i_core_run(core);

	/* TODO:
	 * Perform worker shutdown:
	 *   - migrate queued tasks
	 * */

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
	struct timespec t0, t1;

	clock_gettime(CLOCK_MONOTONIC, &t0);
	status = t->fn(t->arg);
	clock_gettime(CLOCK_MONOTONIC, &t1);

	t->rt = (ilib_uint64_t)(t1.tv_sec - t0.tv_sec) * 1000000000ULL +
		(ilib_uint64_t)(t1.tv_nsec - t0.tv_nsec);

	pthread_mutex_lock(&t->core->lck);
	i_pred_update(&t->core->lpred, t->fn, t->rt);
	pthread_mutex_unlock(&t->core->lck);

	pthread_mutex_lock(&t->sched->lck);
	i_pred_update(&t->sched->gpred, t->fn, t->rt);
	pthread_mutex_unlock(&t->sched->lck);

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

/* prediction private methods */
static int i_pred_init(ipred_t *p)
{
	pthread_mutex_init(&p->lck, NULL);
	return ihstmap_construct(&p->table, NULL, NULL);
}

static int i_pred_destroy(ipred_t *p)
{
	pthread_mutex_destroy(&p->lck);
	return ihstmap_destroy(&p->table);
}

static ilib_uint64_t i_pred_predict(ipred_t *p, int (*fn)(void *), ilib_uint64_t *nsa)
{
	ipred_entry_t *entry;
	pthread_mutex_lock(&p->lck);

	entry = ihstmap_get(&p->table, fn);

	if (entry == NULL) {
		*nsa = 0;
		pthread_mutex_unlock(&p->lck);
		return IDSCHED_DEFAULT_RUNTIME;
	}

	*nsa = entry->nsa;
	pthread_mutex_unlock(&p->lck);

	return entry->ema;
}

static int i_pred_update(ipred_t *p, int (*fn)(void *), ilib_uint64_t rt)
{
	ipred_entry_t *entry;
	pthread_mutex_lock(&p->lck);

	entry = ihstmap_get(&p->table, fn);

	if (entry == NULL) {
		entry = malloc(sizeof(ipred_entry_t));
		if (entry == NULL) {
			pthread_mutex_unlock(&p->lck);
			return -1;
		}

		entry->ema = rt;
		entry->nsa = 1;

		if (ihstmap_insert(&p->table, fn, entry, 0) != 0) {
			free(entry);
			pthread_mutex_unlock(&p->lck);
			return -1;
		}

		pthread_mutex_unlock(&p->lck);

		return 0;
	}

	entry->ema = (ilib_uint64_t)(IDSCHED_EMA_ALPHA * rt + I_ONEMALPHA * entry->ema);
	++entry->nsa;

	pthread_mutex_unlock(&p->lck);
	return 0;
}

/* SYSCALL IMPLEMENTATIONS */

static long i_wait_consider_task(struct iwait_opts *wo)
{
	int retval;
	if (_I__TASK_HAS(wo->wo_target, IDSCHED_TASK_DONE)) {
		wo->wo_stat = wo->wo_target->exitst;

		ilisti_remove(&wo->wo_target->sibling);
		_I__TASK_SET(wo->wo_target, IDSCHED_TASK_REAPED);
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
			_I__TASK_SET(chld, IDSCHED_TASK_REAPED);
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
	idsched_task_t *tsk = i_core_self()->currt;
	int retval;

	wo->wait.task = tsk;
	wo->wait.target = wo->wo_target;

	i_waitqadd(&tsk->wait_chldexit, &wo->wait);

	do {
		retval = i__do_wait(wo);

		if (retval != ISC_WNOWAIT)
			break;

		_I__TASK_STSTAT(tsk, IDSCHED_TASK_BLOCKED);
		i_schedule();
	} while (1);

	i_waitqremove(&tsk->wait_chldexit, &wo->wait);

	_I__TASK_STSTAT(tsk, IDSCHED_TASK_RUNNING);

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

static long i_sys_fork(idsched_task_t *t)
{
	idsched_task_t *prnt = t;
	idsched_core_t *core = t->core;
	idsched_task_t *chld = calloc(1, sizeof(idsched_task_t));
	if (chld == NULL) {
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
	if (chld->ctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld);
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return (long)IDSCHED_INVALID_CHILD;
	}

	chld->shctx.ic_stack.ss_sp = malloc(prnt->shctx.ic_stack.ss_size);
	chld->shctx.ic_stack.ss_flags = 0;
	if (chld->shctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld);
		free(chld->ctx.ic_stack.ss_sp);
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
	chld->pred   = prnt->pred;
	chld->prio   = prnt->prio;
	chld->sched  = prnt->sched;
	chld->exitst = 0;
	chld->rt     = 0;
	chld->ctx.ic_link = &chld->shctx;

	chld->sc_nr  = IDSCHED_SYS_NONE;

	prnt->sc_ret = (long)chld;
	chld->sc_ret = 0;

	chld->prnt   = prnt;
	ilisti_push_back(&prnt->children, &chld->sibling);

	_I__TASK_STSTAT(chld, IDSCHED_TASK_READY);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: FORK enqueue tid=%zu task=%p core=%p\n", chld->tid, (void *)chld, (void *)chld->core);
#endif
	pthread_mutex_lock(&core->lck);
	pqueue_push(&core->rq, &chld);
	pthread_cond_signal(&core->cv);
	pthread_mutex_unlock(&core->lck);

	return (long)chld;
}

static long i_sys_exec(idsched_task_t *t)
{
	int (*fn)(void *) = (void *)t->sc_arg[0];
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
	_I__TASK_STSTAT(t, IDSCHED_TASK_READY);
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
	else if (!(_I__TASK_HAS(t, IDSCHED_TASK_USROWN)))
		i_reaper_enqueue(&t->sched->reaper, t);
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
	i_pred_init(&sched->gpred);

	if (sched->cores == NULL) return -1;

	for (i = 0; i < ncores; ++i) {
		idsched_core_t *core = &sched->cores[i];

		core->sched = sched;
		core->id    = i;
		core->currt = NULL;
		core->flags = IDSCHED_CORE_OFFLINE;

		i_pred_init(&core->lpred);
		pthread_mutex_init(&core->lck, NULL);
		pthread_cond_init(&core->cv, NULL);

		pqueue_construct(&core->rq, sizeof(idsched_task_t *), i_task_cmp);
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

		pqueue_destroy(&core->rq);
		i_pred_destroy(&core->lpred);
		pthread_mutex_destroy(&core->lck);
		pthread_cond_destroy(&core->cv);
	}

	free(sched->cores);
	i_pred_destroy(&sched->gpred);
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

		core->flags = IDSCHED_CORE_ONLINE;

		if (pthread_create(&core->thread, NULL, i_core_worker, core) != 0) {
			continue;
		}
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: BOOTSTRAP awaken Worker Core [%zu]%p", i, core);
#endif
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

		if (core == NULL) {
			pthread_mutex_unlock(&sched->lck);
			break;
		}
		core->flags |= IDSCHED_CORE_STOPPING;
		pthread_mutex_unlock(&sched->lck);

		pthread_mutex_lock(&core->lck);
		pthread_cond_signal(&core->cv);
		pthread_mutex_unlock(&core->lck);
#if IDSCHED_DEBUG == 1
		printf("        -   Worker Core %p\n", core);
		printf("        -   thread %zu waiting for join()\n", core->thread);
#endif
		pthread_join(core->thread, NULL);
#if IDSCHED_DEBUG == 1
		printf("        [X] Worker Core %p joined\n", core);
#endif
		pthread_mutex_lock(&sched->lck);
		core->flags = IDSCHED_CORE_OFFLINE;
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
	_I__TASK_STSTAT(t, IDSCHED_TASK_NEW);
	_I__TASK_SET(t, IDSCHED_TASK_USROWN);
	t->exitst   = 0;
	t->rt       = 0;
	t->pred     = 0;
	t->prio     = 0;

	ilisti_init(&t->children);
	ilisti_init(&t->sibling);

	ilisti_init(&t->wait.node);
	t->wait.task = t;

	pthread_mutex_init(&t->wait_chldexit.lck, NULL);
	ilisti_init(&t->wait_chldexit.head);


	t->ctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->ctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->ctx.ic_stack.ss_flags = 0;

	if (t->ctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		return -1;
	}

	t->shctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->shctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->shctx.ic_stack.ss_flags = 0;

	if (t->shctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		return -1;
	}

	if (igetcontext(&t->ctx) != 0) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		free(t->shctx.ic_stack.ss_sp);
		return -1;
	}

	if (igetcontext(&t->shctx) != 0) {
		pthread_mutex_destroy(&t->wait_chldexit.lck);
		free(t->ctx.ic_stack.ss_sp);
		free(t->shctx.ic_stack.ss_sp);
		return -1;
	}

	t->shctx.ic_link = NULL;

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
	if (t == NULL)
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

	ilib_uint64_t ns;
	t->pred = i_pred_predict(&core->lpred, t->fn, &ns);

	if (ns < IDSCHED_PRED_MIN_HISTORY)
		t->pred = i_pred_predict(&sch->gpred, t->fn, &ns);

	t->prio  = t->pred;
	t->core  = core;
	_I__TASK_STSTAT(t, IDSCHED_TASK_READY);

	t->ctx.ic_link = &t->shctx;

	imakecontext(&t->ctx, (void (*)(void))i_task_entry, 1, t);
	imakecontext(&t->shctx, (void (*)(void))i_sched_entry, 1, t);

	pthread_mutex_lock(&core->lck);
	pqueue_push(&core->rq, &t);
	pthread_cond_signal(&core->cv);
	pthread_mutex_unlock(&core->lck);

	pthread_mutex_unlock(&sch->lck);
	return 0;
}

int idsched_task_wait(int *wstatus)
{
	idsched_core_t *core;
	idsched_task_t *curr;

	core = i_core_self();
	if (core == NULL) return -1;

	curr = core->currt;
	if (curr == NULL) return -1;

	curr->sc_arg[0] = (ilib_uintptr_t)wstatus;

	return (int)i_schedcall(curr, IDSCHED_SYS_WAIT);
}

int idsched_task_waittask(idsched_task_t *t, int *wstatus)
{
	idsched_core_t *core;
	idsched_task_t *curr;

	if (t == NULL)
		return -1;

	core = i_core_self();

	if (core != NULL && (curr = core->currt) != NULL) {
		curr->sc_arg[0] = (ilib_uintptr_t)t;
		curr->sc_arg[1] = (ilib_uintptr_t)wstatus;
		curr->sc_arg[2] = 0;

		return (int)i_schedcall(curr, IDSCHED_SYS_WAIT4);
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
	idsched_task_t *task;

	core = i_core_self();
	if (core == NULL) return -1;

	task = core->currt;
	if (task == NULL) return -1;

	return (int)i_schedcall(task, IDSCHED_SYS_YIELD);
}

int idsched_task_exec(int (*fn)(void *), void *arg)
{
	idsched_core_t *core = i_core_self();
	if (core == NULL) return -1;

	idsched_task_t *t = core->currt;
	if (t == NULL) return -1;

	t->sc_arg[0] = (ilib_uintptr_t)fn;
	t->sc_arg[1] = (ilib_uintptr_t)arg;

	i_schedcall(t, IDSCHED_SYS_EXEC);

	/* should never return */
	return -1;
}

idsched_task_t *idsched_task_fork(void)
{
	idsched_core_t *core = i_core_self();
	idsched_task_t *prnt = core->currt;

	return (idsched_task_t *)i_schedcall(prnt, IDSCHED_SYS_FORK);
}

int idsched_run(idsched_t *sched)
{
	if (sched == NULL) return -1;
	pthread_setspecific(i_core_key, &sched->cores[0]);

	i_core_run(&sched->cores[0]);

	return 0;
}

#endif /* I_IDSCH_IMPL */
#endif /* IDSCHED_IMPLEMENTATION */
