#ifndef GRAPH_H
#define GRAPH_H

#ifndef GRAPH_DECL
#define GRAPH_DECL static inline
#endif /* GRAPH_DECL */

typedef unsigned int vertex_id_t;
typedef unsigned int graph_size_t;

typedef struct {
	vertex_id_t from;
	vertex_id_t to;
} edge_t;

typedef struct {
	vertex_id_t from;
	vertex_id_t to;
	int	    weight;
} wedgei_t;

typedef struct {
	vertex_id_t from;
	vertex_id_t to;
	float	    weight;
} wedgef_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} graph_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;

	vertex_id_t *ioffsets;
	vertex_id_t *iedges;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} dgraph_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;
	int	    *weights;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} wigraph_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;
	int	    *weights;

	vertex_id_t *ioffsets;
	vertex_id_t *iedges;
	int	    *iweights;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} widgraph_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;
	float	    *weights;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} wfgraph_t;

typedef struct {
	vertex_id_t *offsets;
	vertex_id_t *edges;
	float	    *weights;

	vertex_id_t *ioffsets;
	vertex_id_t *iedges;
	float	    *iweights;

	graph_size_t vertex_count;
	graph_size_t edge_count;
} wfdgraph_t;

GRAPH_DECL void graph_construct(graph_t *g, const edge_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void graph_destroy(graph_t *g);

GRAPH_DECL void dgraph_construct(dgraph_t *g, const edge_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void dgraph_destroy(dgraph_t *g);

GRAPH_DECL void wigraph_construct(wigraph_t *g, const wedgei_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void wfgraph_construct(wfgraph_t *g, const wedgef_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void wigraph_destroy(wigraph_t *g);
GRAPH_DECL void wfgraph_destroy(wfgraph_t *g);

GRAPH_DECL void widgraph_construct(widgraph_t *g, const wedgei_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void wfdgraph_construct(wfdgraph_t *g, const wedgef_t *edges, graph_size_t vertex_count, graph_size_t edge_count);
GRAPH_DECL void widgraph_destroy(widgraph_t *g);
GRAPH_DECL void wfdgraph_destroy(wfdgraph_t *g);

typedef struct {
	vertex_id_t *component_id;
	vertex_id_t *postorder;
	graph_size_t component_count;
	char	     has_cycle;
} graph_dfs_result_t;

GRAPH_DECL graph_dfs_result_t graph_dfs_al(const graph_t *g);
GRAPH_DECL graph_dfs_result_t dgraph_dfs_al(const dgraph_t *g);
GRAPH_DECL graph_dfs_result_t wigraph_dfs_al(const wigraph_t *g);
GRAPH_DECL graph_dfs_result_t wfgraph_dfs_al(const wfgraph_t *g);
GRAPH_DECL graph_dfs_result_t widgraph_dfs_al(const widgraph_t *g);
GRAPH_DECL graph_dfs_result_t wfdgraph_dfs_al(const wfdgraph_t *g);

#endif // GRAPH_H

#ifdef GRAPH_IMPLEMENTATION
#ifndef I_GRAPH_IMPLEMENTATION
#define I_GRAPH_IMPLEMENTATION

#if !((defined(GRAPH_MALLOC) == defined(GRAPH_FREE)) && \
      (defined(GRAPH_FREE) == defined(GRAPH_REALLOC)) && \
      (defined(GRAPH_REALLOC) == defined(GRAPH_CALLOC)))
#error "You must either use the default allocator or provide all of: GRAPH_MALLOC, GRAPH_CALLOC, GRAPH_REALLOC and GRAPH_FREE"
#endif

#if !defined(GRAPH_MALLOC) || !defined(GRAPH_FREE) || !defined(GRAPH_REALLOC) || !defined(GRAPH_CALLOC)

#include <stdlib.h>

#ifndef GRAPH_MALLOC
#define GRAPH_MALLOC malloc
#endif /* GRAPH_MALLOC */

#ifndef GRAPH_FREE
#define GRAPH_FREE free
#endif /* GRAPH_FREE */

#ifndef GRAPH_REALLOC
#define GRAPH_REALLOC realloc
#endif /* GRAPH_REALLOC */

#ifndef GRAPH_CALLOC
#define GRAPH_CALLOC calloc
#endif /* GRAPH_CALLOC */

#endif /* !defined(GRAPH_MALLOC) || !defined(GRAPH_FREE) || !defined(GRAPH_REALLOC) */

#if !(defined(GRAPH_MEMSET) == defined(GRAPH_MEMCPY))
#error "You must either use the default mem ops or provide GRAPH_MEMSET and GRAPH_MEMCPY"
#endif

#if !defined(GRAPH_MEMSET) || !defined(GRAPH_MEMCPY)

#include <string.h>

#ifndef GRAPH_MEMSET
#define GRAPH_MEMSET memset
#endif /* GRAPH_MEMSET */

#ifndef GRAPH_MEMCPY
#define GRAPH_MEMCPY memcpy
#endif /* GRAPH_MEMCPY */

#endif /* !defined(GRAPH_MEMSET) || !defined(GRAPH_MEMCPY) */

#define I_BFLAG_REVERSE 1
#define I_BFLAG_TYPEF   2
#define I_DOREVQ(flags) (((flags) & I_BFLAG_REVERSE) != 0)

static inline void i_csrgraph_builder(vertex_id_t *offsets, vertex_id_t *target, const edge_t *edges,
					graph_size_t vertex_count, graph_size_t edge_count, int reverse)
{
	vertex_id_t *cursor;
	graph_size_t i;

	for (i = 0; i < edge_count; i++) {
		vertex_id_t from = reverse ? edges[i].to : edges[i].from;
		offsets[from + 1]++;
	}

	for (i = 0; i < vertex_count; i++) {
		offsets[i + 1] += offsets[i];
	}

	cursor = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	GRAPH_MEMCPY(cursor, offsets, vertex_count * sizeof(vertex_id_t));

	for (i = 0; i < edge_count; i++) {
		vertex_id_t from = reverse ? edges[i].to   : edges[i].from;
		vertex_id_t to   = reverse ? edges[i].from : edges[i].to;
		target[cursor[from]++] = to;
	}

	GRAPH_FREE(cursor);
}

static inline void i_wcsrgraph_builder(vertex_id_t *offsets, vertex_id_t *target, void *out_weights,
					const void *edges, graph_size_t vertex_count, graph_size_t edge_count,
					int flags)
{
	vertex_id_t *cursor;
	graph_size_t i;
	size_t stride;
	int reverse = I_DOREVQ(flags);

	stride = (flags & I_BFLAG_TYPEF) ? sizeof(wedgef_t) : sizeof(wedgei_t);

	for (i = 0; i < edge_count; i++) {
		const char *e = (const char *)edges + i * stride;
		edge_t edge;

		GRAPH_MEMCPY(&edge, e, sizeof(edge));

		offsets[(reverse ? edge.to : edge.from) + 1]++;
	}

	for (i = 0; i < vertex_count; i++)
		offsets[i + 1] += offsets[i];

	cursor = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	GRAPH_MEMCPY(cursor, offsets, vertex_count * sizeof(vertex_id_t));

	for (i = 0; i < edge_count; i++) {
		const char *e = (const char *)edges + i * stride;
		edge_t edge;
		vertex_id_t from, to, pos;

		GRAPH_MEMCPY(&edge, e, sizeof(edge));

		from = reverse ? edge.to : edge.from;
		to = reverse ? edge.from : edge.to;
		pos = cursor[from]++;

		target[pos] = to;

		if (flags & I_BFLAG_TYPEF)
			((float *)out_weights)[pos] = ((const wedgef_t *)e)->weight;
		else
			((int *)out_weights)[pos] = ((const wedgei_t *)e)->weight;
	}

	GRAPH_FREE(cursor);
}

GRAPH_DECL void graph_construct(graph_t *g, const edge_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	edge_t *doubled;
	graph_size_t doubled_count = edge_count * 2;
	graph_size_t i;

	g->vertex_count = vertex_count;
	g->edge_count   = doubled_count;

	doubled = (edge_t *)GRAPH_MALLOC(doubled_count * sizeof(edge_t));
	for (i = 0; i < edge_count; i++) {
		doubled[2 * i].from     = edges[i].from;
		doubled[2 * i].to       = edges[i].to;
		doubled[2 * i + 1].from = edges[i].to;
		doubled[2 * i + 1].to   = edges[i].from;
	}

	g->offsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges   = (vertex_id_t *)GRAPH_MALLOC(doubled_count * sizeof(vertex_id_t));

	i_csrgraph_builder(g->offsets, g->edges, doubled, vertex_count, doubled_count, 0);
	GRAPH_FREE(doubled);
}

GRAPH_DECL void graph_destroy(graph_t *g)
{
	GRAPH_FREE(g->offsets);
	GRAPH_FREE(g->edges);
	g->offsets = NULL;
	g->edges   = NULL;
}

GRAPH_DECL void dgraph_construct(dgraph_t *g, const edge_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	g->vertex_count = vertex_count;
	g->edge_count	= edge_count;

	g->offsets  = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges    = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));
	g->ioffsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->iedges   = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));

	i_csrgraph_builder(g->offsets, g->edges, edges, vertex_count, edge_count, 0);
	i_csrgraph_builder(g->ioffsets, g->iedges, edges, vertex_count, edge_count, 1);
}

GRAPH_DECL void dgraph_destroy(dgraph_t *g)
{
	GRAPH_FREE(g->offsets);
	GRAPH_FREE(g->ioffsets);
	GRAPH_FREE(g->edges);
	GRAPH_FREE(g->iedges);
	g->offsets  = NULL;
	g->ioffsets = NULL;
	g->edges    = NULL;
	g->iedges   = NULL;
}

GRAPH_DECL void wigraph_construct(wigraph_t *g, const wedgei_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	wedgei_t *doubled;
	graph_size_t doubled_count = edge_count * 2;
	graph_size_t i;

	g->vertex_count = vertex_count;
	g->edge_count   = doubled_count;

	doubled = (wedgei_t *)GRAPH_MALLOC(doubled_count * sizeof(wedgei_t));
	for (i = 0; i < edge_count; i++) {
		doubled[2 * i].from     = edges[i].from;
		doubled[2 * i].to       = edges[i].to;
		doubled[2 * i].weight   = edges[i].weight;
		doubled[2 * i + 1].from = edges[i].to;
		doubled[2 * i + 1].to   = edges[i].from;
		doubled[2 * i + 1].weight = edges[i].weight;
	}

	g->offsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges   = (vertex_id_t *)GRAPH_MALLOC(doubled_count * sizeof(vertex_id_t));
	g->weights = (int *)GRAPH_MALLOC(doubled_count * sizeof(int));

	i_wcsrgraph_builder(g->offsets, g->edges, g->weights, doubled, vertex_count, doubled_count, 0);
	GRAPH_FREE(doubled);
}

GRAPH_DECL void wfgraph_construct(wfgraph_t *g, const wedgef_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	wedgef_t *doubled;
	graph_size_t doubled_count = edge_count * 2;
	graph_size_t i;

	g->vertex_count = vertex_count;
	g->edge_count   = doubled_count;

	doubled = (wedgef_t *)GRAPH_MALLOC(doubled_count * sizeof(wedgef_t));
	for (i = 0; i < edge_count; i++) {
		doubled[2 * i].from     = edges[i].from;
		doubled[2 * i].to       = edges[i].to;
		doubled[2 * i].weight   = edges[i].weight;
		doubled[2 * i + 1].from = edges[i].to;
		doubled[2 * i + 1].to   = edges[i].from;
		doubled[2 * i + 1].weight = edges[i].weight;
	}

	g->offsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges   = (vertex_id_t *)GRAPH_MALLOC(doubled_count * sizeof(vertex_id_t));
	g->weights = (float *)GRAPH_MALLOC(doubled_count * sizeof(float));

	i_wcsrgraph_builder(g->offsets, g->edges, g->weights, doubled, vertex_count, doubled_count, I_BFLAG_TYPEF);
	GRAPH_FREE(doubled);
}

#define GRAPH_DEFINE_WGRAPH_DESTROY(name, type) \
GRAPH_DECL void name(type *g) \
{ \
	GRAPH_FREE(g->offsets); \
	GRAPH_FREE(g->edges); \
	GRAPH_FREE(g->weights); \
	g->offsets = NULL; \
	g->edges   = NULL; \
	g->weights = NULL; \
}

GRAPH_DEFINE_WGRAPH_DESTROY(wigraph_destroy, wigraph_t)
GRAPH_DEFINE_WGRAPH_DESTROY(wfgraph_destroy, wfgraph_t)

GRAPH_DECL void widgraph_construct(widgraph_t *g, const wedgei_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	g->vertex_count = vertex_count;
	g->edge_count	= edge_count;

	g->offsets  = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges    = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));
	g->ioffsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->iedges   = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));
	g->weights  = (int *)GRAPH_MALLOC(edge_count * sizeof(int));
	g->iweights = (int *)GRAPH_MALLOC(edge_count * sizeof(int));

	i_wcsrgraph_builder(g->offsets, g->edges, g->weights, edges, vertex_count, edge_count, 0);
	i_wcsrgraph_builder(g->ioffsets, g->iedges, g->iweights, edges, vertex_count, edge_count, I_BFLAG_REVERSE);
}

GRAPH_DECL void wfdgraph_construct(wfdgraph_t *g, const wedgef_t *edges, graph_size_t vertex_count, graph_size_t edge_count)
{
	g->vertex_count = vertex_count;
	g->edge_count	= edge_count;

	g->offsets  = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->edges    = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));
	g->ioffsets = (vertex_id_t *)GRAPH_CALLOC(vertex_count + 1, sizeof(vertex_id_t));
	g->iedges   = (vertex_id_t *)GRAPH_MALLOC(edge_count * sizeof(vertex_id_t));
	g->weights  = (float *)GRAPH_MALLOC(edge_count * sizeof(float));
	g->iweights = (float *)GRAPH_MALLOC(edge_count * sizeof(float));

	i_wcsrgraph_builder(g->offsets, g->edges, g->weights, edges, vertex_count, edge_count, I_BFLAG_TYPEF);
	i_wcsrgraph_builder(g->ioffsets, g->iedges, g->iweights, edges, vertex_count, edge_count, I_BFLAG_REVERSE | I_BFLAG_TYPEF);
}

#define GRAPH_DEFINE_WDGRAPH_DESTROY(name, type) \
GRAPH_DECL void name(type *g) \
{ \
	GRAPH_FREE(g->offsets); \
	GRAPH_FREE(g->edges); \
	GRAPH_FREE(g->weights); \
	GRAPH_FREE(g->ioffsets); \
	GRAPH_FREE(g->iedges); \
	GRAPH_FREE(g->iweights); \
	g->offsets = NULL; \
	g->edges   = NULL; \
	g->weights = NULL; \
	g->ioffsets = NULL; \
	g->iedges   = NULL; \
	g->iweights = NULL; \
}

GRAPH_DEFINE_WDGRAPH_DESTROY(widgraph_destroy, widgraph_t)
GRAPH_DEFINE_WDGRAPH_DESTROY(wfdgraph_destroy, wfdgraph_t)

/* ------------------------------------------------------------------- */

#define I_DFS_UNVISITED ((vertex_id_t)-1)

static inline void
i_dfs_visit_recursive(const vertex_id_t *offsets, const vertex_id_t *edges,
                       vertex_id_t v, vertex_id_t parent, vertex_id_t component,
                       graph_dfs_result_t *result, char *stacked, graph_size_t *postorder_idx)
{
	vertex_id_t i;
	stacked[v] = 1;
	result->component_id[v] = component;

	for (i = offsets[v]; i < offsets[v + 1]; i++) {
		vertex_id_t neighbor = edges[i];

		if (neighbor == parent) continue;

		if (result->component_id[neighbor] == I_DFS_UNVISITED) {
			i_dfs_visit_recursive(offsets, edges, neighbor, v, component, result, stacked, postorder_idx);
		} else if (stacked[neighbor]) {
			result->has_cycle = 1;
		}
	}

	stacked[v] = 0;
	result->postorder[(*postorder_idx)++] = v;
}

static inline graph_dfs_result_t
i_dfs_core(const vertex_id_t *offsets, const vertex_id_t *edges, graph_size_t vertex_count)
{
	graph_dfs_result_t result;
	graph_size_t postorder_idx = 0;
	graph_size_t i;

	char *stacked = (char *)GRAPH_CALLOC(vertex_count, sizeof(char));

	result.component_id = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	result.postorder    = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	result.component_count = 0;
	result.has_cycle = 0;

	for (i = 0; i < vertex_count; i++) {
		result.component_id[i] = I_DFS_UNVISITED;
	}

	for (i = 0; i < vertex_count; i++) {
		if (result.component_id[i] == I_DFS_UNVISITED) {
			i_dfs_visit_recursive(offsets, edges, (vertex_id_t)i, I_DFS_UNVISITED,
			                      (vertex_id_t)result.component_count, &result, stacked, &postorder_idx);
			result.component_count++;
		}
	}

	GRAPH_FREE(stacked);
	return result;
}

#define GRAPH_DEFINE_DFS(name, type) \
GRAPH_DECL graph_dfs_result_t name(const type *g) \
{ return i_dfs_core(g->offsets, g->edges, g->vertex_count); }

GRAPH_DEFINE_DFS(graph_dfs_al, graph_t)
GRAPH_DEFINE_DFS(dgraph_dfs_al, dgraph_t)
GRAPH_DEFINE_DFS(wigraph_dfs_al, wigraph_t)
GRAPH_DEFINE_DFS(wfgraph_dfs_al, wfgraph_t)
GRAPH_DEFINE_DFS(widgraph_dfs_al, widgraph_t)
GRAPH_DEFINE_DFS(wfdgraph_dfs_al, wfdgraph_t)

#endif // I_GRAPH_IMPLEMENTATION
#endif // GRAPH_IMPLEMENTATION

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