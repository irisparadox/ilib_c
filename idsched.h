#ifndef IDSCHED_H_
#define IDSCHED_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif  /* _POSIX_C_SOURCE */

#include <unistd.h>
#include <pthread.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L
#error "idsched.h needs a POSIX-compliant (POSIX.1-2008) system (pthreads, unistd.h)"
#endif /* !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L */

#include "deftypei.h"
#include "rbtree.h"
#include "queue.h"

#ifndef IDSCHED_EMA_ALPHA
#define IDSCHED_EMA_ALPHA 0.2
#endif  /* IDSCHED_EMA_ALPHA */

#ifndef IDSCHED_DEFAULT_DURATION
#define IDSCHED_DEFAULT_DURATION 1.0
#endif /* IDSCHED_DEFAULT_DURATION */

#define IDSCHED_BLOCKED 0
#define IDSCHED_READY   1
#define IDSCHED_RUNNING 2
#define IDSCHED_DONE    3

typedef struct idsched_predictor {
	rb_node_t       node;
	void          (*fn)(void *arg);
	double          ema;
	double          ema_var;
	unsigned long   n_samples;
} idsched_predictor_t;

typedef struct idsched_task {
	void (*fn)(void *arg);
	void  *arg;

	struct idsched_task **succ;
	ilib_size_t           succ_len;
	ilib_size_t           succ_cap;

	struct idsched_task **pred;
	ilib_size_t           pred_len;
	ilib_size_t           pred_cap;

	int unmet_deps;
	unsigned char state;

	double duration;
	double cp;
	int cp_computed;
} idsched_task_t;

typedef struct idsched_core {
	pqueue_t        ready;

	pthread_t       thread;
	pthread_mutex_t lock;

	int             active;
} idsched_core_t;

typedef struct idsched {
	idsched_core_t *cores;
	ilib_size_t     cores_cap;

	rb_tree_t       rbpd;
	pthread_mutex_t rbpdlock;
} idsched_t;

idsched_t      *idsched_create(ilib_size_t max_cores);
void            idsched_destroy(idsched_t *sched);

int             idsched_add_core(idsched_t *sched);
int             idsched_remove_core(idsched_t *sched, ilib_size_t core_id);

idsched_task_t *idsched_task_create(idsched_t* sched, void (*fn)(void *arg), void *arg);
void            idsched_task_destroy(idsched_task_t *t);
void            idsched_task_destroy_all(idsched_task_t **tasks, ilib_size_t n_tasks);
int             idsched_add_dep(idsched_task_t *t, idsched_task_t *depends_on);

idsched_task_t *idsched_pick_next(idsched_task_t **tasks, ilib_size_t n_tasks);
void            idsched_report_done(idsched_t *sched, idsched_task_t *t, double elapsed);
void            idsched_run_all(idsched_t *sched, idsched_task_t **tasks, ilib_size_t n_tasks);

#endif /* IDSCHED_H_ */

#define IDSCHED_IMPLEMENTATION
#ifdef IDSCHED_IMPLEMENTATION

#include <stdlib.h>
#include <time.h>

#ifndef RBTREE_IMPLEMENTATION
#define RBTREE_IMPLEMENTATION
#endif  /* RBTREE_IMPLEMENTATION */

#ifndef PQUEUE_IMPLEMENTATION
#define PQUEUE_IMPLEMENTATION
#endif  /* PQUEUE_IMPLEMENTATION */

#include "rbtree.h"
#include "queue.h"

#define I_ONEMALPHA (1 - IDSCHED_EMA_ALPHA)

static int i_core_cmp(const void *a, const void *b)
{
	idsched_task_t *ta, *tb;

	ta = *(idsched_task_t * const *)a;
	tb = *(idsched_task_t * const *)b;

	return (tb->cp > ta->cp) - (tb->cp < ta->cp);
}

static int i_pred_cmp(const rb_node_t *a, const rb_node_t *b)
{
	idsched_predictor_t *sa, *sb;
	ilib_uintptr_t       pa,  pb;

	sa = RB_CONTAINER(a, idsched_predictor_t, node);
	sb = RB_CONTAINER(b, idsched_predictor_t, node);

	pa = (ilib_uintptr_t)(void (*)(void *))sa->fn;
	pb = (ilib_uintptr_t)(void (*)(void *))sb->fn;

	return (pa > pb) - (pa < pb);
}

static int i_pred_kcmp(const void *key, const rb_node_t *n)
{
	void (*fn)(void *arg);
	idsched_predictor_t *s;
	ilib_uintptr_t pk, ps;

	fn = *(void (**)(void *arg))key;
	s  = RB_CONTAINER(n, idsched_predictor_t, node);

	pk = (ilib_uintptr_t)fn;
	ps = (ilib_uintptr_t)(void (*)(void *))s->fn;

	return (pk > ps) - (pk < ps);
}

static void i_mark_dirty(idsched_task_t *t)
{
	ilib_size_t i;

	if (!t->cp_computed) return;

	t->cp_computed = 0;
	for (i = 0; i < t->pred_len; ++i)
		i_mark_dirty(t->pred[i]);
}

static int i_arr_grow(idsched_task_t ***arr, ilib_size_t *len, ilib_size_t *cap)
{
	idsched_task_t **tmp;
	ilib_size_t      newcap;

	if (*len < *cap) return 1;

	newcap = (*cap == 0) ? 4 : (*cap * 2);
	tmp    = realloc(*arr, newcap * sizeof(*tmp));
	if (!tmp) return 0;

	*arr = tmp;
	*cap = newcap;

	return 1;
}

static double i_compute_cp(idsched_task_t *t)
{
	double best, c;
	ilib_size_t i;

	if (t->cp_computed) return t->cp;

	best = 0.0;
	for (i = 0; i < t->succ_len; ++i) {
		c = i_compute_cp(t->succ[i]);
		if (c > best)
			best = c;
	}

	t->cp = t->duration + best;
	t->cp_computed = 1;

	return t->cp;
}

static void i_predictor_update(idsched_t *sched, void (*fn)(void *arg), double actual)
{
	idsched_predictor_t *st;
	rb_node_t           *n;
	double               delta;

	n = rb_search(&sched->rbpd, &fn);
	if (n) {
		st = RB_CONTAINER(n, idsched_predictor_t, node);
	} else {
		st = malloc(sizeof(idsched_predictor_t));
		if (!st) return; /* TODO error handling */
		st->fn        = fn;
		st->ema       = actual;
		st->ema_var   = 0.0;
		st->n_samples = 0;
		rb_insert(&sched->rbpd, &st->node);
	}

	if (st->n_samples > 0) {
		delta    = actual - st->ema;
		st->ema += IDSCHED_EMA_ALPHA * delta;
		st->ema_var = I_ONEMALPHA * (st->ema_var + IDSCHED_EMA_ALPHA * delta * delta);
	}

	st->n_samples++;
}

static void i_core_migrate_all(idsched_t *sched, ilib_size_t fromidx, ilib_size_t toidx)
{
	idsched_core_t *from, *to;
	idsched_task_t *t;

	from = &sched->cores[fromidx];
	to   = &sched->cores[toidx];

	if (fromidx < toidx) {
		pthread_mutex_lock(&from->lock);
		pthread_mutex_lock(&to->lock);
	} else {
		pthread_mutex_lock(&to->lock);
		pthread_mutex_lock(&from->lock);
	}

	while (!pqueue_empty(&from->ready)) {
		t = pqueue_top(&from->ready, idsched_task_t *);
		pqueue_pop(&from->ready);
		pqueue_push(&to->ready, &t);
	}

	pthread_mutex_unlock(&from->lock);
	pthread_mutex_unlock(&to->lock);
}

idsched_t *idsched_create(ilib_size_t max_cores)
{
	idsched_t   *sched;
	ilib_size_t  i;

	sched = malloc(sizeof(idsched_t));
	if (!sched) return NULL;

	sched->cores = calloc(max_cores, sizeof(idsched_core_t));
	if (!sched->cores) {
		free(sched);
		return NULL;
	}
	sched->cores_cap = max_cores;

	for (i = 0; i < max_cores; ++i) {
		sched->cores[i].active = 0;
	}

	rb_init(&sched->rbpd, i_pred_cmp, i_pred_kcmp);
	pthread_mutex_init(&sched->rbpdlock, NULL);

	return sched;
}

void idsched_destroy(idsched_t *sched)
{
	rb_node_t           *n;
	idsched_predictor_t *st;

	/* TODO active cores should be shutdown first */

	while (!rb_empty(&sched->rbpd)) {
		n  = rb_minimum(sched->rbpd.root);
		st = RB_CONTAINER(n, idsched_predictor_t, node);
		rb_delete(&sched->rbpd, n);
		free(st);
	}

	pthread_mutex_destroy(&sched->rbpdlock);
	free(sched->cores);
	free(sched);
}


int idsched_add_core(idsched_t *sched)
{
	ilib_size_t i;

	for (i = 0; i < sched->cores_cap; ++i) {
		if (sched->cores[i].active) continue;

		if (pqueue_construct(&sched->cores[i].ready, sizeof(idsched_task_t *), i_core_cmp) != 0)
			return -1;

		pthread_mutex_init(&sched->cores[i].lock, NULL);
		sched->cores[i].active = 1;

		return (int)i;
	}

	return -1;
}

int idsched_remove_core(idsched_t *sched, ilib_size_t core_id)
{
	ilib_size_t i;
	int         dest;

	if (core_id >= sched->cores_cap || !sched->cores[core_id].active)
		return -1;

	if (!pqueue_empty(&sched->cores[core_id].ready)) {
		dest = -1;
		for (i = 0; i < sched->cores_cap; ++i) {
			if (i != core_id && sched->cores[i].active) {
				dest = (int)i;
				break;
			}
		}

		if (dest < 0) return -1;

		i_core_migrate_all(sched, core_id, (ilib_size_t)dest);
	}

	pqueue_destroy(&sched->cores[core_id].ready);
	pthread_mutex_destroy(&sched->cores[core_id].lock);
	sched->cores[core_id].active = 0;

	return 0;
}

idsched_task_t *idsched_task_create(idsched_t *sched, void (*fn)(void *), void *arg)
{
	idsched_task_t      *t;
	idsched_predictor_t *st;
	rb_node_t           *n;

	t = malloc(sizeof(idsched_task_t));
	if (!t) return NULL;

	t->fn          = fn;
	t->arg         = arg;
	t->succ        = NULL;
	t->succ_len    = 0;
	t->succ_cap    = 0;
	t->pred        = NULL;
	t->pred_len    = 0;
	t->pred_cap    = 0;
	t->unmet_deps  = 0;
	t->state       = IDSCHED_READY;
	t->cp_computed = 0;

	n = rb_search(&sched->rbpd, &fn);
	if (n) {
		st = RB_CONTAINER(n, idsched_predictor_t, node);
		t->duration = st->ema;
	} else {
		t->duration = IDSCHED_DEFAULT_DURATION;
	}

	return t;
}

void idsched_task_destroy(idsched_task_t *t)
{
	free(t->succ);
	free(t->pred);
	free(t);
}

void idsched_task_destroy_all(idsched_task_t **tasks, ilib_size_t n_tasks)
{
	ilib_size_t i;

	for (i = 0; i < n_tasks; i++)
		idsched_task_destroy(tasks[i]);
}

int idsched_add_dep(idsched_task_t *t, idsched_task_t *depends_on)
{
	if (!i_arr_grow(&depends_on->succ, &depends_on->succ_len, &depends_on->succ_cap))
		return 0;

	if (!i_arr_grow(&t->pred, &t->pred_len, &t->pred_cap))
		return 0;

	depends_on->succ[depends_on->succ_len++] = t;
	t->pred[t->pred_len++] = depends_on;
	t->unmet_deps++;
	t->state = IDSCHED_BLOCKED;

	return 1;
}

idsched_task_t *idsched_pick_next(idsched_task_t **tasks, ilib_size_t n_tasks)
{
	ilib_size_t     i;
	idsched_task_t *t, *best;
	double          c;

	best = NULL;
	for (i = 0; i < n_tasks; ++i) {
		t = tasks[i];
		if (t->state != IDSCHED_READY) continue;

		c = i_compute_cp(t);
		if (!best || c > best->cp)
			best = t;
	}

	if (best)
		best->state = IDSCHED_RUNNING;

	return best;
}

void idsched_report_done(idsched_t *sched, idsched_task_t *t, double elapsed)
{
	ilib_size_t i;
	idsched_task_t *s;

	i_predictor_update(sched, t->fn, elapsed);

	t->duration = elapsed;
	i_mark_dirty(t);

	t->state = IDSCHED_DONE;

	for (i = 0; i < t->succ_len; ++i) {
		s = t->succ[i];
		s->unmet_deps--;
		if (s->unmet_deps == 0)
			s->state = IDSCHED_READY;
	}
}

void idsched_run_all(idsched_t *sched, idsched_task_t **tasks, ilib_size_t n_tasks)
{
	idsched_task_t *t;
	struct timespec t0, t1;
	double elapsed;

	for (;;) {
		t = idsched_pick_next(tasks, n_tasks);
		if (!t) break;

		clock_gettime(CLOCK_MONOTONIC, &t0);
		t->fn(t->arg);
		clock_gettime(CLOCK_MONOTONIC, &t1);

		elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
		idsched_report_done(sched, t, elapsed);
	}
}

#endif /* IDSCHED_IMPLEMENTATION  */
