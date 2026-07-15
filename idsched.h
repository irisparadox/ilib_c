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

#include "deftypei.h"
#include "queue.h"

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

#define IDSCHED_WIFEXITED(status)   ((status) >= 0)
#define IDSCHED_WEXITSTATUS(status) (status)
#define IDSCHED_INVALID_TID ((ilib_uint64_t)-1)

typedef struct idsched      idsched_t;
typedef struct idsched_acpi idsched_acpi_t;
typedef struct idsched_core idsched_core_t;
typedef struct idsched_task idsched_task_t;

typedef ilib_uint64_t idsched_tid;

struct idsched {
	ilib_size_t     ncores;
	ilib_uint64_t   nseq;
	idsched_core_t *cores;

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

	idsched_task_t *currt;

	ilib_uint32_t   flags;
};

struct idsched_task {
	int  (*fn)(void *);
	void  *arg;

	idsched_t      *sched;
	idsched_core_t *core;

	ilib_uint32_t   flags;
	idsched_tid     tid;

	int status;

	pthread_mutex_t lck;
	pthread_cond_t  cv;
};

int idsched_create(idsched_t *sched, ilib_size_t ncores);
int idsched_destroy(idsched_t *sched);

int idsched_core_startup(idsched_t *sched, ilib_size_t n);
int idsched_core_shutdown(idsched_t *sched, ilib_size_t n);

idsched_tid idsched_task_create(idsched_t *sch, idsched_task_t *t, int (*fn)(void *), void *arg);
int         idsched_task_submit(idsched_t *sch, idsched_task_t *t);
int         idsched_task_wait(idsched_task_t *t, int *status);
int         idsched_task_destroy(idsched_task_t *t);

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

static int i_task_cmp(const void *a, const void *b)
{
	const idsched_task_t *ta;
	const idsched_task_t *tb;

	ta = *(idsched_task_t * const *)a;
	tb = *(idsched_task_t * const *)b;

	return (ta->tid > tb->tid) - (ta->tid < tb->tid);
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

		t = pqueue_top(&core->rq, idsched_task_t *);
		pqueue_pop(&core->rq);

		pthread_mutex_lock(&t->lck);
		t->flags    = IDSCHED_TASK_RUNNING;
		core->currt = t;

		pthread_mutex_unlock(&core->lck);
		int status = t->fn(t->arg);
		pthread_mutex_lock(&core->lck);
		t->status   = status;
		t->flags    = IDSCHED_TASK_DONE;
		pthread_cond_broadcast(&t->cv);

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

	i_core_run(core);

	/* TODO:
	 * Perform worker shutdown:
	 *   - migrate queued tasks
	 * */

	return NULL;
}

/* Checks for bootstrap */

static int i_is_bst_core(idsched_t *sched)
{
	return pthread_equal(pthread_self(), sched->cores[0].thread);
}

static idsched_core_t *i_core_shtdwn_n(idsched_t *sched)
{
	ilib_size_t     i;
	idsched_core_t *core = NULL;

	for (i = 1; i < sched->ncores; ++i) {
		pthread_mutex_lock(&sched->cores[i].lck);
		if (sched->cores[i].flags & IDSCHED_CORE_ONLINE) {
			core = &sched->cores[i];
			pthread_mutex_unlock(&sched->cores[i].lck);

			break;
		}
		pthread_mutex_unlock(&sched->cores[i].lck);
	}

	return core;
}

static ilib_size_t i_cntr_flag(idsched_t *sched, ilib_uint32_t flag)
{
	ilib_size_t i;
	ilib_size_t cnt = 0;
	for (i = 1; i < sched->ncores; ++i) {
		pthread_mutex_lock(&sched->cores[i].lck);
		if (sched->cores[i].flags & flag)
			++cnt;
		pthread_mutex_unlock(&sched->cores[i].lck);
	}
	return cnt;
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
	sched->cores  = calloc(ncores, sizeof(idsched_core_t));

	if (sched->cores == NULL) return -1;

	for (i = 0; i < ncores; ++i) {
		idsched_core_t *core = &sched->cores[i];

		core->sched = sched;
		core->id    = i;
		core->currt = NULL;
		core->flags = IDSCHED_CORE_OFFLINE;

		pthread_mutex_init(&core->lck, NULL);
		pthread_cond_init(&core->cv, NULL);

		pqueue_construct(&core->rq, sizeof(idsched_task_t *), i_task_cmp);
	}

	sched->cores[0].flags  = IDSCHED_CORE_ONLINE;
	sched->cores[0].thread = pthread_self();

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

		pthread_mutex_destroy(&core->lck);
		pthread_cond_destroy(&core->cv);
	}

	free(sched->cores);

	pthread_mutex_destroy(&sched->lck);
	sched->cores  = NULL;
	sched->ncores = 0;
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

	offline = i_cntr_flag(sched, IDSCHED_CORE_OFFLINE);

	if (n == IDSCHED_ALL_CORES || n > offline)
		n = offline;

	started = 0;

	for (i = 1; i < sched->ncores && started < n; ++i) {
		core = &sched->cores[i];

		pthread_mutex_lock(&core->lck);
		if (!(core->flags & IDSCHED_CORE_OFFLINE)) {
			pthread_mutex_unlock(&core->lck);
			continue;
		}

		core->flags = IDSCHED_CORE_ONLINE;

		if (pthread_create(&core->thread, NULL, i_core_worker, core) != 0) {
			pthread_mutex_unlock(&core->lck);
			continue;
		}
		pthread_mutex_unlock(&core->lck);
		++started;
	}

	return (int)started;
}

int idsched_core_shutdown(idsched_t *sched, ilib_size_t n)
{
	idsched_core_t *core;
	ilib_size_t     online;
	ilib_size_t     stopped;

	if (sched == NULL || !i_is_bst_core(sched)) return -1;

	online = i_cntr_flag(sched, IDSCHED_CORE_ONLINE);

	if (n == IDSCHED_ALL_CORES || n > online)
		n = online;

	stopped = 0;

	while (stopped < n) {
		core = i_core_shtdwn_n(sched);

		if (core == NULL) break;

		pthread_mutex_lock(&core->lck);

		core->flags |= IDSCHED_CORE_STOPPING;
		pthread_cond_signal(&core->cv);

		pthread_mutex_unlock(&core->lck);

		pthread_join(core->thread, NULL);

		pthread_mutex_lock(&core->lck);

		core->flags = IDSCHED_CORE_OFFLINE;

		pthread_mutex_unlock(&core->lck);

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
	t->sched  = sch;
	t->core   = NULL;
	t->flags  = IDSCHED_TASK_NEW;
	t->status = 0;

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
	ilib_size_t     i, online, target, seen;

	if (sch == NULL || t == NULL) return -1;
	if (!(t->flags & IDSCHED_TASK_NEW)) return -1;

	online = 0;
	for (i = 1; i < sch->ncores; ++i) {
		pthread_mutex_lock(&sch->cores[i].lck);
		if ((sch->cores[i].flags & IDSCHED_CORE_ONLINE) &&
		    !(sch->cores[i].flags & IDSCHED_CORE_STOPPING))
			++online;
		pthread_mutex_unlock(&sch->cores[i].lck);
	}

	if (online == 0) return -1;

	target = t->tid % online;

	core = NULL;
	seen = 0;

	for (i = 1; i < sch->ncores; ++i) {
		pthread_mutex_lock(&sch->cores[i].lck);

		if ((sch->cores[i].flags & IDSCHED_CORE_ONLINE) &&
		    !(sch->cores[i].flags & IDSCHED_CORE_STOPPING)) {
			if (seen == target) {
				core = &sch->cores[i];
				break;
			}
			++seen;
		}

		pthread_mutex_unlock(&sch->cores[i].lck);
	}

	if (core == NULL) return -1;

	t->core  = core;
	t->flags = IDSCHED_TASK_READY;

	pqueue_push(&core->rq, &t);
	pthread_cond_signal(&core->cv);

	pthread_mutex_unlock(&core->lck);

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

	return 0;
}

int idsched_run(idsched_t *sched)
{
	if (sched == NULL) return -1;

	i_core_run(&sched->cores[0]);
	return 0;
}

#endif /* I_IDSCH_IMPL */
#endif /* IDSCHED_IMPLEMENTATION */
