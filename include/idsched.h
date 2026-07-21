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

typedef struct iwaitq {
	pthread_mutex_t lck;
	ilinode_t       head;
} iwaitq;

typedef struct ireaper {
	pthread_t       thread;

	pthread_mutex_t lck;
	pthread_cond_t  cv;

	ilinode_t       zombies;

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

struct idsched_core {
	idsched_t      *sched;
	ilib_size_t     id;

	pthread_t       thread;

	pthread_mutex_t lck;
	pthread_cond_t  cv;

	pqueue_t        rq;
	ipred_t         lpred;

	idsched_task_t *currt;
	icontext_t      shctx;

	ilib_uint32_t   flags;
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

#define IDSCHED_INVALID_CHILD ((idsched_task_t *)-1)

#define IDSCHED_SYS_NONE  0
#define IDSCHED_SYS_FORK  1
#define IDSCHED_SYS_YIELD 2
#define IDSCHED_SYS_WAIT  3
#define IDSCHED_SYS_EXIT  4
#define IDSCHED_SYS_EXEC  5
#define IDSCHED_SYS_WAITT 6

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
static void i_fix_frame_chain(const idsched_task_t *prnt, idsched_task_t *chld)
{
	ilib_uintptr_t plow;
	ilib_uintptr_t phigh;
	ilib_uintptr_t clow;
	imreg_t       *rbp;
	imreg_t        saved;
	ilib_uintptr_t off;

	plow  = (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;
	phigh = plow + prnt->ctx.ic_stack.ss_size;
	clow  = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp;

	rbp = (imreg_t *)chld->ctx.ic_mcontext.gregs[IMREG_RBP];

	while (1) {
		if ((ilib_uintptr_t)rbp < clow ||
		    (ilib_uintptr_t)rbp >= clow + chld->ctx.ic_stack.ss_size)
			break;

		saved = *rbp;

		if ((ilib_uintptr_t)saved < plow ||
		    (ilib_uintptr_t)saved >= phigh)
			break;

		off  = (ilib_uintptr_t)saved - plow;
		*rbp = (imreg_t)(clow + off);
		rbp  = (imreg_t *)*rbp;
	}
}

static void i_relocate_stack(idsched_task_t *prnt, idsched_task_t *chld)
{
	ilib_uintptr_t old_base;
	ilib_uintptr_t new_base;
	ilib_intptr_t delta;

	old_base = (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;
	new_base = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp;

	delta = (ilib_intptr_t)(new_base - old_base);

	ilib_uintptr_t *ptr = (ilib_uintptr_t *)new_base;

	ilib_size_t words =
		chld->ctx.ic_stack.ss_size / sizeof(ilib_uintptr_t);

	for (ilib_size_t i = 0; i < words; ++i, ++ptr) {
		ilib_uintptr_t val = *ptr;

		/*
		 * Pointer into parent's stack.
		 */
		if (val >= old_base &&
		    val < old_base + prnt->ctx.ic_stack.ss_size) {
			*ptr = (ilib_uintptr_t)((ilib_intptr_t)val + delta);
			continue;
		}

		/*
		 * Fix task self references.
		 */
		if (val == (ilib_uintptr_t)prnt) {
			*ptr = (ilib_uintptr_t)chld;
#if IDSCHED_DEBUG == 1
			printf("relocate_stack: fix task self references with val %p -> %p\n", (void *)val, (void *)chld);
#endif
		}
	}
}
#endif /* defined(__x86_64__) */

/* SCHEDULER-SPACE & SYSCALLS */
static void i_sys_fork(idsched_task_t *t);
static void i_sys_exec(idsched_task_t *t);
static void i_sys_yield(idsched_task_t *t);
static void i_sys_wait(idsched_task_t *t);
static void i_sys_waittask(idsched_task_t *t);
static void i_sys_exit(idsched_task_t *t);

static int             i_reaper_init(ireaper *r, idsched_t *sch);
static void            i_reaper_destroy(ireaper *r);
static void            i_reaper_enqueue(ireaper *r, idsched_task_t *t);
static idsched_task_t *i_reaper_dequeue(ireaper *r);
static void           *i_reaper_main(void *arg);

static int i_dispatch_schedcall(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("dispatch %d for tid=%zu\n", t->sc_nr, t->tid);
#endif
	switch(t->sc_nr) {
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
	case IDSCHED_SYS_WAITT:
		if (t->sc_nr == IDSCHED_SYS_WAIT)
			i_sys_wait(t);
		else
			i_sys_waittask(t);

		if (t->flags & IDSCHED_TASK_BLOCKED)
			return IDSCHED_DISPATCH_STOP;

		return IDSCHED_DISPATCH_CONTINUE;
	case IDSCHED_SYS_EXIT:
		i_sys_exit(t);
		return IDSCHED_DISPATCH_STOP;
	default:
		return IDSCHED_DISPATCH_CONTINUE;
	}
}

static long i_schedcall(idsched_task_t *t, int nr)
{
#if IDSCHED_DEBUG == 1
	printf("schedcall tid=%zu nr=%d\n", t->tid, nr);
	printf("schedcall arg t=%p currt=%p\n",
		(void *)t,
	       (void *)i_core_self()->currt);
#endif
	t->sc_nr  = nr;

	iswapcontext(&t->ctx, &t->core->shctx);

	return i_core_self()->currt->sc_ret;
}

static void i_waitqadd(iwaitq *wq, iwaitq_entry *we)
{
	pthread_mutex_lock(&wq->lck);

	ilisti_push_back(&wq->head, &we->node);

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

		we->task->flags = IDSCHED_TASK_READY;

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
	printf("[DEBUG_SCHED]: Reaper enqueued and signaled\n");
	printf("[DEBUG_SCHED]: Node is %s\n", t ? "Task" : "Poison Pill");
	printf("[DEBUG_SCHED]: Node pointer %p\n", t);
#endif
	pthread_mutex_unlock(&r->lck);
}

static idsched_task_t *i_reaper_dequeue(ireaper *r)
{
	pthread_mutex_lock(&r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper dequeue in progress\n");
#endif
	while (ilisti_empty(&r->zombies))
		pthread_cond_wait(&r->cv, &r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper awoken to kill zombie\n");
#endif
	ilinode_t *zombn = ilisti_front(&r->zombies);
	ilisti_remove(zombn);

	pthread_mutex_unlock(&r->lck);
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper zombie pointer %p\n", zombn);
#endif
	if (zombn == &r->poison) return NULL;

	idsched_task_t *t = ILISTI_ENTRY(zombn, idsched_task_t, sibling);
	return t;
}

static void *i_reaper_main(void *arg)
{
	ireaper        *r;
	idsched_task_t *t;

	r = arg;
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper main loop start\n");
#endif
	for (;;) {
		t = i_reaper_dequeue(r); /* This will return NULL if destruction.  */

		/* Death can die too.  */
		if (t == NULL) break;

#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: Reaper killing %p\n", t);
#endif
		idsched_task_destroy(t);
		free(t);
#if IDSCHED_DEBUG == 1
		printf("[DEBUG_SCHED]: Reaper killed task\n");
#endif
	}
#if IDSCHED_DEBUG == 1
	printf("[DEBUG_SCHED]: Reaper exiting\n");
#endif
	return NULL;
}

/* Main loop */
static void i_core_run(idsched_core_t *core)
{
	idsched_task_t *t;
	pthread_mutex_lock(&core->lck);

	for (;;) {
		while (pqueue_empty(&core->rq) && !(core->flags & IDSCHED_CORE_STOPPING))
			pthread_cond_wait(&core->cv, &core->lck);

		if (pqueue_empty(&core->rq) && (core->flags & IDSCHED_CORE_STOPPING))
			break;

		t = pqueue_top(&core->rq, idsched_task_t *);
		pqueue_pop(&core->rq);
#if IDSCHED_DEBUG == 1
		printf("scheduler: running tid=%zu\n", t->tid);
#endif

		pthread_mutex_lock(&t->lck);
		t->flags    = IDSCHED_TASK_RUNNING;
		pthread_mutex_unlock(&t->lck);
		core->currt = t;

		pthread_mutex_unlock(&core->lck);

		for (;;) {
#if IDSCHED_DEBUG == 1
			printf("RIP=%p\n",
				(void *)t->ctx.ic_mcontext.gregs[IMREG_RIP]);

			printf("RSP=%p\n",
				(void *)t->ctx.ic_mcontext.gregs[IMREG_RSP]);
#endif
			iswapcontext(&core->shctx, &t->ctx);
#if IDSCHED_DEBUG == 1
			printf("scheduler: returned from tid=%zu\n", t->tid);
			printf("sc_nr=%d flags=%x\n", t->sc_nr, t->flags);
#endif
			if (t->sc_nr == IDSCHED_SYS_NONE)
				break;

			switch(i_dispatch_schedcall(t)) {
			case IDSCHED_DISPATCH_CONTINUE:
				t->sc_nr = IDSCHED_SYS_NONE;
				continue;
			case IDSCHED_DISPATCH_STOP:
			default:
				t->sc_nr = IDSCHED_SYS_NONE;
				goto done_task;
			}
		}

	done_task:

		pthread_mutex_lock(&core->lck);
		pthread_mutex_lock(&t->lck);

		if (t->flags & IDSCHED_TASK_READY)
			pqueue_push(&core->rq, &t);

		pthread_mutex_unlock(&t->lck);

		core->currt = NULL;
	}

	pthread_mutex_unlock(&core->lck);
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

static void i_sys_wait(idsched_task_t *t)
{
	ilinode_t *pos;
	ilinode_t *tmp;
	idsched_task_t *chld;
	int *status;

	status = (int *)t->sc_arg[0];

	for (;;) {
		ILISTI_FOREACH_SAFE(pos, tmp, &t->children) {
			chld = ILISTI_ENTRY(pos, idsched_task_t, sibling);

			if (!(chld->flags & IDSCHED_TASK_DONE))
				continue;

			if (status != NULL)
				*status = chld->exitst;

			ilisti_remove(&chld->sibling);
			chld->flags |= IDSCHED_TASK_REAPED;
			i_reaper_enqueue(&t->sched->reaper, chld);

			t->sc_ret = 0;
			return;
		}

		if (ilisti_empty(&t->children)) {
			t->sc_ret = -1;
			return;
		}

		ilisti_init(&t->wait.node);
		t->wait.task   = t;
		t->wait.target = NULL;

		i_waitqadd(&t->wait_chldexit, &t->wait);

		t->flags  = IDSCHED_TASK_BLOCKED;
		t->sc_ret = 0;
		return;
	}
}

static void i_sys_waittask(idsched_task_t *t)
{
	idsched_task_t *chld;
	int            *status;
	int             options;

	chld    = (idsched_task_t *)t->sc_arg[0];
	status  = (int *)t->sc_arg[1];
	options = (int)t->sc_arg[2];

	(void)options;

	if (chld == NULL || chld->prnt != t) {
		t->sc_ret = -1;
		return;
	}

	if (chld->flags & IDSCHED_TASK_DONE) {
		if (status != NULL)
			*status = chld->exitst;

		ilisti_remove(&chld->sibling);
		chld->flags |= IDSCHED_TASK_REAPED;
		i_reaper_enqueue(&t->sched->reaper, chld);

		t->sc_ret = 0;
		return;
	}

	ilisti_init(&t->wait.node);
	t->wait.task   = t;
	t->wait.target = chld;
	i_waitqadd(&t->wait_chldexit, &t->wait);

	t->flags = IDSCHED_TASK_BLOCKED;
	t->sc_ret = 0;
}

static void i_sys_fork(idsched_task_t *t)
{
	idsched_task_t *prnt = t;
	idsched_core_t *core = t->core;
	idsched_task_t *chld = malloc(sizeof(idsched_task_t));
	if (chld == NULL) {
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return;
	}

	pthread_mutex_init(&chld->lck, NULL);
	pthread_cond_init(&chld->cv, NULL);

	pthread_mutex_lock(&core->sched->lck);
	chld->tid = core->sched->nseq++;
	pthread_mutex_unlock(&core->sched->lck);

	memcpy(&chld->ctx, &prnt->ctx, sizeof(icontext_t));

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
	if (chld->ctx.ic_stack.ss_sp == NULL) {
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld);
		prnt->sc_ret = (imreg_t)IDSCHED_INVALID_CHILD;
		return;
	}

	chld->ctx.ic_stack.ss_size = prnt->ctx.ic_stack.ss_size;

	memcpy(chld->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_size);

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
	i_relocate_stack(prnt, chld);
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

#if defined(__x86_64__)
	i_fix_frame_chain(prnt, chld);
#endif /* defined(__x86_64__) */

	chld->fn     = prnt->fn;
	chld->arg    = prnt->arg;

	ilisti_init(&chld->children);
	ilisti_init(&chld->sibling);

	chld->wait.task = chld;
	ilisti_init(&chld->wait.node);

	pthread_mutex_init(&chld->wait_chldexit.lck, NULL);
	ilisti_init(&chld->wait_chldexit.head);

	chld->prnt   = prnt;

	ilisti_push_back(&prnt->children, &chld->sibling);

	chld->core   = prnt->core;
	chld->pred   = prnt->pred;
	chld->prio   = prnt->prio;
	chld->sched  = prnt->sched;
	chld->exitst = 0;
	chld->rt     = 0;
	chld->flags  = IDSCHED_TASK_READY;
	chld->ctx.ic_link = &core->shctx;

	chld->sc_nr  = IDSCHED_SYS_NONE;

	prnt->sc_ret = (long)chld;
	chld->sc_ret = 0;

	pthread_mutex_lock(&core->lck);
	pqueue_push(&core->rq, &chld);
	pthread_cond_signal(&core->cv);
	pthread_mutex_unlock(&core->lck);
}

static void i_sys_exec(idsched_task_t *t)
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

static void i_sys_yield(idsched_task_t *t)
{
	pthread_mutex_lock(&t->lck);
	t->flags = IDSCHED_TASK_READY;
	pthread_mutex_unlock(&t->lck);

	t->sc_ret = 0;
}

static void i_sys_exit(idsched_task_t *t)
{
#if IDSCHED_DEBUG == 1
	printf("EXIT syscall tid=%zu\n", t->tid);
#endif
	int status;

	status = (int)t->sc_arg[0];

	t->exitst = status;
	t->flags  = IDSCHED_TASK_DONE;

	pthread_mutex_lock(&t->lck);
	pthread_cond_broadcast(&t->cv);
	pthread_mutex_unlock(&t->lck);

	if (t->prnt != NULL)
		i_waitqwakeone(&t->prnt->wait_chldexit, t);
	t->sc_ret = 0;
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

	pthread_mutex_lock(&sched->lck);
	online = sched->online;

	if (n == IDSCHED_ALL_CORES || n > online)
		n = online;

	pthread_mutex_unlock(&sched->lck);
	stopped = 0;

	while (stopped < n) {
		pthread_mutex_lock(&sched->lck);
		core = i_core_shtdwn_n(sched);

		if (core == NULL) {
			pthread_mutex_unlock(&sched->lck);
			break;
		}
		core->flags |= IDSCHED_CORE_STOPPING;
		pthread_mutex_unlock(&sched->lck);

		pthread_mutex_lock(&core->lck);
		pthread_cond_signal(&core->cv);
		pthread_mutex_unlock(&core->lck);

		pthread_join(core->thread, NULL);

		pthread_mutex_lock(&sched->lck);
		core->flags = IDSCHED_CORE_OFFLINE;
		--sched->online;
		pthread_mutex_unlock(&sched->lck);

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
	t->flags    = IDSCHED_TASK_NEW;
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

	if (igetcontext(&t->ctx) != 0) {
		free(t->ctx.ic_stack.ss_sp);
		return -1;
	}

	t->ctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->ctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->ctx.ic_stack.ss_flags = 0;

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
	if (!(t->flags & IDSCHED_TASK_DONE)) {
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
	t->flags  = IDSCHED_TASK_NEW;
	t->tid    = 0;
	t->exitst = 0;

	free(t->ctx.ic_stack.ss_sp);
	t->ctx.ic_stack.ss_sp   = NULL;
	t->ctx.ic_stack.ss_size = 0;

	return 0;
}

int idsched_task_submit(idsched_t *sch, idsched_task_t *t)
{
	idsched_core_t *core;
	ilib_size_t     i, target, seen;

	if (sch == NULL || t == NULL) return -1;
	if (!(t->flags & IDSCHED_TASK_NEW)) return -1;

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
	t->flags = IDSCHED_TASK_READY;

	t->ctx.ic_link = &core->shctx;

	imakecontext(&t->ctx, (void (*)(void))i_task_entry, 1, t);

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

	if (t == NULL) return -1;

	core = i_core_self();

	if (core != NULL && (curr = core->currt) != NULL) {
		curr->sc_arg[0] = (ilib_uintptr_t)t;
		curr->sc_arg[1] = (ilib_uintptr_t)wstatus;
		curr->sc_arg[2] = 0;

		return (int)i_schedcall(curr, IDSCHED_SYS_WAITT);
	}

	pthread_mutex_lock(&t->lck);

	while (!(t->flags & IDSCHED_TASK_DONE))
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
