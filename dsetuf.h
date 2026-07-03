/*
 * dsetuf.h -- single-header disjoint-set (union-find)
 *
 * SPDX-License-Identifier: MIT
 *
 * Index-based disjoint-set structure operating on integer elements
 * in [0, size). Uses full path compression + union by size.
 *
 * Usage:
 *     #define DSUF_IMPLEMENTATION
 *     #include "dsetuf.h"
 *
 * FUNCTIONS
 *     dset_construct     Initialize a disjoint_set_t of the given size,
 *                        each element starting in its own singleton set.
 *
 *     dset_destroy       Free all memory owned by a disjoint_set_t.
 *
 *     dset_find          Return the representative (root) of the set
 *                        containing x, compressing the path to it.
 *
 *     dset_join          Merge the sets containing a and b (union by size).
 *
 *     dset_joined        Return whether a and b belong to the same set.
 *
 *     dset_cardinality   Return the number of elements in the set
 *                        containing x.
 */

#ifndef DSETUF_H
#define DSETUF_H

#ifndef DSUF_DECL
#define DSUF_DECL static inline
#endif /* DSUF_DECL */

typedef unsigned int set_size_t;

typedef struct {
	set_size_t  set_count; /* number of disjoint sets */
	set_size_t *size;      /* size of each tree-set */
	int	   *parent;    /* basically parent_of(x) */
} disjoint_set_t;

DSUF_DECL void dset_construct(disjoint_set_t *ds, set_size_t size);
DSUF_DECL void dset_destroy(disjoint_set_t *ds);

DSUF_DECL int dset_find(disjoint_set_t *ds, int x);
DSUF_DECL void dset_join(disjoint_set_t *ds, int a, int b);
DSUF_DECL int dset_joined(const disjoint_set_t *ds, int a, int b);
DSUF_DECL set_size_t dset_cardinality(const disjoint_set_t *ds, int x);

#endif /* DSETUF_H */

#define DSUF_IMPLEMENTATION
#ifdef DSUF_IMPLEMENTATION
#ifndef I_DSUF_IMPLEMENTATION
#define I_SETUF_IMPLEMENTATION

#if !(defined(DSUF_MALLOC) == defined(DSUF_FREE))
#error "You must either use the default allocator or provide all of: DSUF_MALLOC and DSUF_FREE"
#endif

#if !defined(DSUF_MALLOC) || !defined(DSUF_FREE)

#include <stdlib.h>

#ifndef DSUF_MALLOC
#define DSUF_MALLOC malloc
#endif /* DSUF_MALLOC */

#ifndef DSUF_FREE
#define DSUF_FREE free
#endif /* DSUF_FREE */

#endif /* !defined(DSUF_MALLOC) || !defined(DSUF_FREE) */

DSUF_DECL void dset_construct(disjoint_set_t *ds, set_size_t size)
{
	ds->set_count = size;
	ds->parent    = (int *)DSUF_MALLOC(size * sizeof(int));
	ds->size      = (set_size_t *)DSUF_MALLOC(size * sizeof(set_size_t));

	set_size_t i;
	for (i = 0; i < size; ++i) {
		ds->parent[i] = i;
		ds->size[i]   = 1;
	}
}

DSUF_DECL void dset_destroy(disjoint_set_t *ds)
{
	ds->set_count = 0;
	DSUF_FREE(ds->parent);
	DSUF_FREE(ds->size);

	ds->parent = NULL;
	ds->size   = NULL;
}

DSUF_DECL int dset_find(disjoint_set_t *ds, int x)
{
	if (ds->parent[x] == x) return x;
	else return ds->parent[x] = dset_find(ds, ds->parent[x]);
}

DSUF_DECL void dset_join(disjoint_set_t *ds, int a, int b)
{
	int i = dset_find(ds, a);
	int j = dset_find(ds, b);
	if (i == j) return;
	if (ds->size[i] > ds->size[j]) {
		ds->size[i] += ds->size[j];
		ds->parent[j] = i;
	} else {
		ds->size[j] += ds->size[i];
		ds->parent[i] = j;
	}

	ds->set_count--;
}

DSUF_DECL int dset_joined(const disjoint_set_t *ds, int a, int b)
{
	return dset_find((disjoint_set_t *)ds, a) == dset_find((disjoint_set_t *)ds, b);
}

DSUF_DECL set_size_t dset_cardinality(const disjoint_set_t *ds, int x)
{
	return ds->size[dset_find((disjoint_set_t *)ds, x)];
}

#endif /* I_DSUF_IMPLEMENTATION */
#endif /* DSUF_IMPLEMENTATION */

/*
 * MIT License
 *
 * Copyright (c) 2026 Gabriel Guillermo Saavedra Moreira
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */