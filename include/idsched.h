#ifndef IDSCHED_H_
#define IDSCHED_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif  /* _POSIX_C_SOURCE */

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L
#error "idsched.h needs a POSIX-compliant (POSIX.1-2008) system (pthreads, unistd.h)"
#endif /* !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L */

#include <deftypei.h>
#include <queue.h>
#include <ihstmap.h>
#include <icontext.h>

#define IDSCHED_CORE_OFFLINE  (1u << 0)
#define IDSCHED_CORE_ONLINE   (1u << 1)
#define IDSCHED_CORE_STOPPING (1u << 2)
#define IDSCHED_CORE_IDLE     (1u << 3)

#define IDSCHED_TASK_NEW      (1u << 0)
#define IDSCHED_TASK_READY    (1u << 1)
#define IDSCHED_TASK_RUNNING  (1u << 2)
#define IDSCHED_TASK_DONE     (1u << 3)
#define IDSCHED_TASK_BLOCKED  (1u << 4)

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

typedef ilib_uint64_t idsched_tid;

struct idsched {
	ilib_size_t     ncores;
	ilib_size_t     online;

	ilib_uint64_t   nseq;
	idsched_core_t *cores;

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

struct idsched_task {
	int  (*fn)(void *);
	void  *arg;
	idsched_task_t *prnt;

	icontext_t      ctx;

	idsched_t      *sched;
	idsched_core_t *core;

	ilib_uint32_t   flags;
	idsched_tid     tid;

	int status;

	ilib_uint64_t   rt;    /* runtime */
	ilib_uint64_t   pred;  /* prediction */
	ilib_uint64_t   prio;  /* priority */

	pthread_mutex_t lck;
	pthread_cond_t  cv;
};

#define IDSCHED_INVALID_CHILD ((idsched_task_t *)-1)

int idsched_create(idsched_t *sched, ilib_size_t ncores);
int idsched_destroy(idsched_t *sched);

int idsched_core_startup(idsched_t *sched, ilib_size_t n);
int idsched_core_shutdown(idsched_t *sched, ilib_size_t n);

idsched_tid     idsched_task_create(idsched_t *sch, idsched_task_t *t, int (*fn)(void *), void *arg);
int             idsched_task_destroy(idsched_task_t *t);
int             idsched_task_submit(idsched_t *sch, idsched_task_t *t);
int             idsched_task_wait(idsched_task_t *t, int *status);
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

	idc = (ta->tid < tb->tid) -
	      (ta->tid > tb->tid);

	return pc != 0 ? pc : idc;
}

/* ---------------- PRIVATE METHODS ---------------- */

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

		printf("scheduler: queue size=%zu\n", core->rq.size);

		t = pqueue_top(&core->rq, idsched_task_t *);
		pqueue_pop(&core->rq);

		pthread_mutex_lock(&t->lck);
		t->flags    = IDSCHED_TASK_RUNNING;
		pthread_mutex_unlock(&t->lck);
		core->currt = t;

		pthread_mutex_unlock(&core->lck);

		printf("scheduler: switching to tid=%u\n", (unsigned)t->tid);
		fflush(stdout);

		iswapcontext(&core->shctx, &t->ctx);

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
	int status;
	struct timespec t0, t1;

	clock_gettime(CLOCK_MONOTONIC, &t0);
	status = t->fn(t->arg);
	clock_gettime(CLOCK_MONOTONIC, &t1);

	pthread_mutex_lock(&t->lck);
	t->status = status;
	t->flags  = IDSCHED_TASK_DONE;
	t->rt     = (ilib_uint64_t)(t1.tv_sec - t0.tv_sec) * 1000000000ULL
		    + (ilib_uint64_t)(t1.tv_nsec - t0.tv_nsec);

	i_pred_update(&t->core->lpred, t->fn, t->rt);
	i_pred_update(&t->sched->gpred, t->fn, t->rt);
	pthread_cond_broadcast(&t->cv);
	pthread_mutex_unlock(&t->lck);
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

	return 0;
}

int idsched_destroy(idsched_t *sched)
{
	ilib_size_t i;

	if (sched == NULL) return -1;

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

	t->fn     = fn;
	t->arg    = arg;
	t->prnt   = NULL;
	t->sched  = sch;
	t->core   = NULL;
	t->flags  = IDSCHED_TASK_NEW;
	t->status = 0;
	t->rt     = 0;
	t->pred   = 0;
	t->prio   = 0;

	if (igetcontext(&t->ctx) != 0) {
		free(t->ctx.ic_stack.ss_sp);
		return -1;
	}

	t->ctx.ic_stack.ss_sp    = malloc(IDSCHED_STCK_SIZE);
	t->ctx.ic_stack.ss_size  = IDSCHED_STCK_SIZE;
	t->ctx.ic_stack.ss_flags = 0;


	pthread_mutex_init(&t->lck, NULL);
	pthread_cond_init(&t->cv, NULL);

	pthread_mutex_lock(&sch->lck);
	t->tid = sch->nseq++;
	pthread_mutex_unlock(&sch->lck);

	return t->tid;
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

int idsched_task_wait(idsched_task_t *t, int *status)
{
	if (t == NULL) return -1;

	pthread_mutex_lock(&t->lck);

	while (!(t->flags & IDSCHED_TASK_DONE))
		pthread_cond_wait(&t->cv, &t->lck);

	if (status != NULL)
		*status = t->status;

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

	pthread_mutex_lock(&task->lck);
	task->flags = IDSCHED_TASK_READY;
	pthread_mutex_unlock(&task->lck);

	iswapcontext(&task->ctx, &core->shctx);

	return 0;
}

int idsched_task_destroy(idsched_task_t *t)
{
	if (t == NULL || idsched_task_wait(t, NULL) != 0)
		return -1;

	pthread_mutex_destroy(&t->lck);
	pthread_cond_destroy(&t->cv);

	t->fn     = NULL;
	t->arg    = NULL;
	t->sched  = NULL;
	t->core   = NULL;
	t->flags  = IDSCHED_TASK_NEW;
	t->tid    = 0;
	t->status = 0;

	free(t->ctx.ic_stack.ss_sp);
	t->ctx.ic_stack.ss_sp   = NULL;
	t->ctx.ic_stack.ss_size = 0;

	return 0;
}

int idsched_task_exec(int (*fn)(void *), void *arg)
{
	idsched_core_t *core = i_core_self();
	if (core == NULL) return -1;

	idsched_task_t *t = core->currt;
	if (t == NULL) return -1;

	pthread_mutex_lock(&t->lck);
	t->fn  = fn;
	t->arg = arg;
	pthread_mutex_unlock(&t->lck);

	imakecontext(&t->ctx, (void (*)(void))i_task_entry, 1, t);
	isetcontext(&t->ctx);

	return -1;
}

idsched_task_t *idsched_task_fork(void)
{
	idsched_core_t *core = i_core_self();
	idsched_task_t *prnt = core->currt;

	igetcontext(&prnt->ctx);

	idsched_task_t *chld = malloc(sizeof(idsched_task_t));
	if (chld == NULL) return IDSCHED_INVALID_CHILD;

	pthread_mutex_init(&chld->lck, NULL);
	pthread_cond_init(&chld->cv, NULL);

	pthread_mutex_lock(&core->sched->lck);
	chld->tid = core->sched->nseq++;
	pthread_mutex_unlock(&core->sched->lck);

	memcpy(&chld->ctx, &prnt->ctx, sizeof(icontext_t));

	chld->ctx.ic_stack.ss_sp = malloc(prnt->ctx.ic_stack.ss_size);
	if (chld->ctx.ic_stack.ss_sp == NULL) {
		printf("fork: ss_sp malloc failed!\n");
		pthread_mutex_destroy(&chld->lck);
		pthread_cond_destroy(&chld->cv);
		free(chld);
		return IDSCHED_INVALID_CHILD;
	}

	chld->ctx.ic_stack.ss_size = prnt->ctx.ic_stack.ss_size;

	memcpy(chld->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_sp,
	       prnt->ctx.ic_stack.ss_size);

	ilib_uintptr_t off = prnt->ctx.ic_mcontext.gregs[IMREG_RSP] -
			     (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;

	chld->ctx.ic_mcontext.gregs[IMREG_RSP] = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp + off;

	off = prnt->ctx.ic_mcontext.gregs[IMREG_RBX] - (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;

	chld->ctx.ic_mcontext.gregs[IMREG_RBX] = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp + off;

	off = prnt->ctx.ic_mcontext.gregs[IMREG_RBP] - (ilib_uintptr_t)prnt->ctx.ic_stack.ss_sp;

	chld->ctx.ic_mcontext.gregs[IMREG_RBP] = (ilib_uintptr_t)chld->ctx.ic_stack.ss_sp + off;

	chld->fn     = prnt->fn;
	chld->arg    = prnt->arg;
	chld->core   = prnt->core;
	chld->pred   = prnt->pred;
	chld->prio   = prnt->prio;
	chld->sched  = prnt->sched;
	chld->status = 0;
	chld->rt     = 0;

	chld->flags = IDSCHED_TASK_READY;

	chld->ctx.ic_mcontext.gregs[IMREG_RAX] = (imreg_t)NULL;
	chld->ctx.ic_link = &core->shctx;

	pthread_mutex_lock(&core->lck);
	pqueue_push(&core->rq, &chld);

	printf("fork: queued child tid=%u\n", (unsigned)chld->tid);
	printf("fork: parent tid=%u\n", (unsigned)prnt->tid);
	printf("fork: queue size=%zu\n", core->rq.size);

	pthread_cond_signal(&core->cv);
	pthread_mutex_unlock(&core->lck);

	return chld;
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
