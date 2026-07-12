#ifndef IDSCHED_H_
#define IDSCHED_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif  /* _POSIX_C_SOURCE */

#include <unistd.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L
#error "idsched.h needs a POSIX-compliant (POSIX.1-2008) system (pthreads, unistd.h)"
#endif /* !defined(_POSIX_VERSION) || _POSIX_VERSION < 200809L */

#include "deftypei.h"

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
	int done;
} idsched_task_t;

idsched_task_t *idsched_task_create(void (*fn)(void *arg), void *arg);
int             idsched_add_dep(idsched_task_t *t, idsched_task_t *depends_on);

#endif /* IDSCHED_H_ */

#define IDSCHED_IMPLEMENTATION
#ifdef IDSCHED_IMPLEMENTATION

#include <stdlib.h>

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

idsched_task_t *idsched_task_create(void (*fn)(void *), void *arg)
{
	idsched_task_t *t;

	t = malloc(sizeof(idsched_task_t));
	if (!t) return NULL;

	t->fn         = fn;
	t->arg        = arg;
	t->succ       = NULL;
	t->succ_len   = 0;
	t->succ_cap   = 0;
	t->pred       = NULL;
	t->pred_len   = 0;
	t->pred_cap   = 0;
	t->unmet_deps = 0;
	t->done       = 0;

	return t;
}

int idsched_add_dep(idsched_task_t *t, idsched_task_t *depends_on)
{
	/* TODO rollback on failures */

	if (!i_arr_grow(&depends_on->succ, &depends_on->succ_len, &depends_on->succ_cap))
		return 0;

	if (!i_arr_grow(&t->pred, &t->pred_len, &t->pred_cap))
		return 0;

	depends_on->succ[depends_on->succ_len++] = t;
	t->pred[t->pred_len++] = depends_on;
	t->unmet_deps++;

	return 1;
}

#endif /* IDSCHED_IMPLEMENTATION  */
