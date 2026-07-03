/* SPDX-License-Identifier: MIT */
/*
 * Create undirected, directed, weighted, and unweighted graphs
 * easily, and perform common graph algorithms with a simple C API.
 *
 * In exactly one translation unit define GRAPH_IMPLEMENTATION
 * before including to emit function definitions. All other
 * translation units include normally and get declarations.
 *
 * Define GRAPH_DECL before including to override default linkage.
 */

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

GRAPH_DECL edge_t   *graph_edges_al(const graph_t *g, graph_size_t *out_count);
GRAPH_DECL edge_t   *dgraph_edges_al(const dgraph_t *g, graph_size_t *out_count);
GRAPH_DECL wedgei_t *wigraph_edges_al(const wigraph_t *g, graph_size_t *out_count);
GRAPH_DECL wedgei_t *widgraph_edges_al(const widgraph_t *g, graph_size_t *out_count);
GRAPH_DECL wedgef_t *wfgraph_edges_al(const wfgraph_t *g, graph_size_t *out_count);
GRAPH_DECL wedgef_t *wfdgraph_edges_al(const wfdgraph_t *g, graph_size_t *out_count);

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

typedef struct {
	char 	     *visited;
	vertex_id_t  *prev;
	graph_size_t *dist;
	vertex_id_t   origin;
} graph_bfs_result_t;

GRAPH_DECL graph_bfs_result_t graph_bfs_al(const graph_t *g, vertex_id_t origin);
GRAPH_DECL graph_bfs_result_t dgraph_bfs_al(const dgraph_t *g, vertex_id_t origin);

typedef struct { vertex_id_t vertex; int   dist; } pq_entry_i_t;
typedef struct { vertex_id_t vertex; float dist; } pq_entry_f_t;

typedef struct {
	int         *dist;
	vertex_id_t *prev;
	char        *visited;
	vertex_id_t  origin;
} graph_dijkstra_i_result_t;

typedef struct {
	float       *dist;
	vertex_id_t *prev;
	char        *visited;
	vertex_id_t  origin;
} graph_dijkstra_f_result_t;

GRAPH_DECL graph_dijkstra_i_result_t wigraph_dijkstra_al(const wigraph_t *g, vertex_id_t origin);
GRAPH_DECL graph_dijkstra_i_result_t widgraph_dijkstra_al(const widgraph_t *g, vertex_id_t origin);
GRAPH_DECL graph_dijkstra_f_result_t wfgraph_dijkstra_al(const wfgraph_t *g, vertex_id_t origin);
GRAPH_DECL graph_dijkstra_f_result_t wfdgraph_dijkstra_al(const wfdgraph_t *g, vertex_id_t origin);

typedef struct {
	int           span_weight;
	wedgei_t     *MST;
	graph_size_t  mst_size;
} graph_kruskal_i_result_t;

typedef struct {
	float         span_weight;
	wedgef_t     *MST;
	graph_size_t  mst_size;
} graph_kruskal_f_result_t;

GRAPH_DECL graph_kruskal_i_result_t wigraph_kruskal_al(const wigraph_t *g);
GRAPH_DECL graph_kruskal_f_result_t wfgraph_kruskal_al(const wfgraph_t *g);

#endif // GRAPH_H

//#define GRAPH_IMPLEMENTATION
#ifdef GRAPH_IMPLEMENTATION
#ifndef I_GRAPH_IMPLEMENTATION
#define I_GRAPH_IMPLEMENTATION

#ifndef QUEUE_IMPLEMENTATION
#ifndef PQUEUE_IMPLEMENTATION
#define PQUEUE_IMPLEMENTATION
#endif /* PQUEUE_IMPLEMENTATION */
#define QUEUE_IMPLEMENTATION
#include "queue.h"
#endif

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

#endif /* !defined(GRAPH_MALLOC) || !defined(GRAPH_FREE) || !defined(GRAPH_REALLOC) || !defined(GRAPH_CALLOC) */

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

static void i_csrgraph_builder(vertex_id_t *offsets, vertex_id_t *target, const edge_t *edges,
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

static void i_wcsrgraph_builder(vertex_id_t *offsets, vertex_id_t *target, void *out_weights,
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

/*
 * X-macro generator for per-graph-type edge-list extraction.
 *
 * weight_assign - statement (no trailing ;) assigning edges[count].weight,
 *                 or (void)0 for unweighted types.
 * dedup_guard   - condition; `v > u` for undirected types (take each edge
 *                 once), `1` for directed types (every CSR entry is a
 *                 distinct real edge, take them all).
 */
#define GRAPH_DEFINE_EDGES_AL(fn_name, graph_type, edge_type, weight_assign, dedup_guard) \
GRAPH_DECL edge_type *fn_name(const graph_type *g, graph_size_t *out_count) \
{ \
	edge_type   *edges; \
	graph_size_t count; \
	vertex_id_t  u; \
	vertex_id_t  v; \
	vertex_id_t  i; \
\
	edges = (edge_type *)GRAPH_MALLOC(sizeof(edge_type) * g->edge_count); \
	count = 0; \
\
	for (u = 0; u < g->vertex_count; ++u) { \
		for (i = g->offsets[u]; i < g->offsets[u + 1]; ++i) { \
			v = g->edges[i]; \
			if (dedup_guard) { \
				edges[count].from = u; \
				edges[count].to   = v; \
				weight_assign; \
				count++; \
			} \
		} \
	} \
\
	*out_count = count; \
	return edges; \
}

GRAPH_DEFINE_EDGES_AL(graph_edges_al,    graph_t,    edge_t,   (void)0,                             v > u)
GRAPH_DEFINE_EDGES_AL(dgraph_edges_al,   dgraph_t,   edge_t,   (void)0,                             1)
GRAPH_DEFINE_EDGES_AL(wigraph_edges_al,  wigraph_t,  wedgei_t, edges[count].weight = g->weights[i], v > u)
GRAPH_DEFINE_EDGES_AL(widgraph_edges_al, widgraph_t, wedgei_t, edges[count].weight = g->weights[i], 1)
GRAPH_DEFINE_EDGES_AL(wfgraph_edges_al,  wfgraph_t,  wedgef_t, edges[count].weight = g->weights[i], v > u)
GRAPH_DEFINE_EDGES_AL(wfdgraph_edges_al, wfdgraph_t, wedgef_t, edges[count].weight = g->weights[i], 1)

/* ------------------------------------------------------------------- */

#define I_VTX_UNVISITED ((vertex_id_t)-1)

static void
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

		if (result->component_id[neighbor] == I_VTX_UNVISITED) {
			i_dfs_visit_recursive(offsets, edges, neighbor, v, component, result, stacked, postorder_idx);
		} else if (stacked[neighbor]) {
			result->has_cycle = 1;
		}
	}

	stacked[v] = 0;
	result->postorder[(*postorder_idx)++] = v;
}

static graph_dfs_result_t
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
		result.component_id[i] = I_VTX_UNVISITED;
	}

	for (i = 0; i < vertex_count; i++) {
		if (result.component_id[i] == I_VTX_UNVISITED) {
			i_dfs_visit_recursive(offsets, edges, (vertex_id_t)i, I_VTX_UNVISITED,
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

static graph_bfs_result_t
i_bfs_core(const vertex_id_t *offsets, const vertex_id_t *edges, graph_size_t vertex_count, vertex_id_t origin)
{
	graph_bfs_result_t result;
	graph_size_t i;
	queue_t q;

	result.visited = (char *)GRAPH_MALLOC(vertex_count * sizeof(char));
	result.prev    = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	result.dist    = (graph_size_t *)GRAPH_MALLOC(vertex_count * sizeof(graph_size_t));
	result.origin  = origin;

	for (i = 0; i < vertex_count; ++i) {
		result.visited[i] = 0;
		result.prev[i]    = I_VTX_UNVISITED;
		result.dist[i]    = I_VTX_UNVISITED;
	}

	queue_construct(&q, sizeof(vertex_id_t));

	result.visited[origin] = 1;
	result.dist[origin]    = 0;

	i_queue_push(&q, &origin);

	while (!queue_empty(&q)) {
		vertex_id_t v = queue_front(&q, vertex_id_t); queue_pop(&q);
		vertex_id_t j;
		
		for (j = offsets[v]; j < offsets[v + 1]; ++j) {
			vertex_id_t neighbor = edges[j];
			if (!result.visited[neighbor]) {
				result.visited[neighbor] = 1;
				result.prev[neighbor]	 = v;
				result.dist[neighbor]	 = result.dist[v] + 1;
				i_queue_push(&q, &neighbor);
			}
		}
	}

	queue_destroy(&q);
	return result;
}

#define GRAPH_DEFINE_BFS(name, type) \
GRAPH_DECL graph_bfs_result_t name(const type *g, vertex_id_t origin) \
{ return i_bfs_core(g->offsets, g->edges, g->vertex_count, origin); }

GRAPH_DEFINE_BFS(graph_bfs_al, graph_t)
GRAPH_DEFINE_BFS(dgraph_bfs_al, dgraph_t)

PQ_DEFINE_CMP_FIELD_ASC(pq_cmp_entry_i_asc, pq_entry_i_t, dist)
PQ_DEFINE_CMP_FIELD_ASC(pq_cmp_entry_f_asc, pq_entry_f_t, dist)

#define I_DIJKSTRA_INT_INF ((int)0x7fffffff)
#define I_DIJKSTRA_FLT_INF (3.402823466e+38f)

static graph_dijkstra_i_result_t
i_dijkstra_i_core(const vertex_id_t *offsets, const vertex_id_t *edges, const int *weights,
                   graph_size_t vertex_count, vertex_id_t origin)
{
	graph_dijkstra_i_result_t result;
	pqueue_t pq;
	graph_size_t i;
	result.dist    = (int *)GRAPH_MALLOC(vertex_count * sizeof(int));
	result.prev    = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	result.visited = (char *)GRAPH_MALLOC(vertex_count * sizeof(char));
	result.origin  = origin;
	for (i = 0; i < vertex_count; i++) {
		result.dist[i]    = I_DIJKSTRA_INT_INF;
		result.prev[i]    = I_VTX_UNVISITED;
		result.visited[i] = 0;
	}
	result.dist[origin] = 0;
	pqueue_construct(&pq, sizeof(pq_entry_i_t), pq_cmp_entry_i_asc);
	pqueue_push(&pq, &(pq_entry_i_t){ .vertex = origin, .dist = 0 });
	while (!pqueue_empty(&pq)) {
		pq_entry_i_t top = pqueue_top(&pq, pq_entry_i_t);
		vertex_id_t u = top.vertex;
		vertex_id_t j;
		pqueue_pop(&pq);
		if (result.visited[u]) {
			continue; /* stale entry, a better path was already finalized */
		}
		result.visited[u] = 1;
		for (j = offsets[u]; j < offsets[u + 1]; j++) {
			vertex_id_t v = edges[j];
			int w = weights[j];
			int alt = result.dist[u] + w;
			if (!result.visited[v] && alt < result.dist[v]) {
				result.dist[v] = alt;
				result.prev[v] = u;
				pqueue_push(&pq, &(pq_entry_i_t){ .vertex = v, .dist = alt });
			}
		}
	}
	pqueue_destroy(&pq);
	return result;
}

static graph_dijkstra_f_result_t
i_dijkstra_f_core(const vertex_id_t *offsets, const vertex_id_t *edges, const float *weights,
                   graph_size_t vertex_count, vertex_id_t origin)
{
	graph_dijkstra_f_result_t result;
	pqueue_t pq;
	graph_size_t i;
	result.dist    = (float *)GRAPH_MALLOC(vertex_count * sizeof(float));
	result.prev    = (vertex_id_t *)GRAPH_MALLOC(vertex_count * sizeof(vertex_id_t));
	result.visited = (char *)GRAPH_MALLOC(vertex_count * sizeof(char));
	result.origin  = origin;
	for (i = 0; i < vertex_count; i++) {
		result.dist[i]    = I_DIJKSTRA_FLT_INF;
		result.prev[i]    = I_VTX_UNVISITED;
		result.visited[i] = 0;
	}
	result.dist[origin] = 0.0f;
	pqueue_construct(&pq, sizeof(pq_entry_f_t), pq_cmp_entry_f_asc);
	pqueue_push(&pq, &(pq_entry_f_t){ .vertex = origin, .dist = 0.0f });
	while (!pqueue_empty(&pq)) {
		pq_entry_f_t top = pqueue_top(&pq, pq_entry_f_t);
		vertex_id_t u = top.vertex;
		vertex_id_t j;
		pqueue_pop(&pq);
		if (result.visited[u]) {
			continue;
		}
		result.visited[u] = 1;
		for (j = offsets[u]; j < offsets[u + 1]; j++) {
			vertex_id_t v = edges[j];
			float w = weights[j];
			float alt = result.dist[u] + w;
			if (!result.visited[v] && alt < result.dist[v]) {
				result.dist[v] = alt;
				result.prev[v] = u;
				pqueue_push(&pq, &(pq_entry_f_t){ .vertex = v, .dist = alt });
			}
		}
	}
	pqueue_destroy(&pq);
	return result;
}

GRAPH_DECL graph_dijkstra_i_result_t wigraph_dijkstra_al(const wigraph_t *g, vertex_id_t origin)
{ return i_dijkstra_i_core(g->offsets, g->edges, g->weights, g->vertex_count, origin); }

GRAPH_DECL graph_dijkstra_i_result_t widgraph_dijkstra_al(const widgraph_t *g, vertex_id_t origin)
{ return i_dijkstra_i_core(g->offsets, g->edges, g->weights, g->vertex_count, origin); }

GRAPH_DECL graph_dijkstra_f_result_t wfgraph_dijkstra_al(const wfgraph_t *g, vertex_id_t origin)
{ return i_dijkstra_f_core(g->offsets, g->edges, g->weights, g->vertex_count, origin); }

GRAPH_DECL graph_dijkstra_f_result_t wfdgraph_dijkstra_al(const wfdgraph_t *g, vertex_id_t origin)
{ return i_dijkstra_f_core(g->offsets, g->edges, g->weights, g->vertex_count, origin); }

#ifndef DSUF_IMPLEMENTATION
#define DSUF_IMPLEMENTATION

#include "dsetuf.h"
#endif /* DSUF_IMPLEMENTATION */

PQ_DEFINE_CMP_FIELD_ASC(kruskal_cmpi, wedgei_t, weight);
PQ_DEFINE_CMP_FIELD_ASC(kruskal_cmpf, wedgef_t, weight);

GRAPH_DECL graph_kruskal_i_result_t wigraph_kruskal_al(const wigraph_t *g)
{
	pqueue_t          pq;
	wedgei_t         *edges;
	graph_size_t      edge_count;
	graph_size_t      k;
	vertex_id_t       u;
	vertex_id_t       v;
	wedgei_t          e;
	disjoint_set_t    ds;
	graph_kruskal_i_result_t result = {0};

	edges = wigraph_edges_al(g, &edge_count);

	pqueue_construct(&pq, sizeof(wedgei_t), kruskal_cmpi);
	for (k = 0; k < edge_count; ++k) {
		pqueue_push(&pq, &edges[k]);
	}
	GRAPH_FREE(edges);

	dset_construct(&ds, g->vertex_count);
	result.MST = (wedgei_t *)GRAPH_MALLOC((g->vertex_count - 1) * sizeof(wedgei_t));

	while (!pqueue_empty(&pq)) {
		e = pqueue_top(&pq, wedgei_t); pqueue_pop(&pq);
		u = e.from; v = e.to;
		if (!dset_joined(&ds, u, v)) {
			dset_join(&ds, u, v);
			result.span_weight += e.weight;
			result.MST[result.mst_size++] = e;
			if (result.mst_size == g->vertex_count - 1) break;
		}
	}

	pqueue_destroy(&pq);
	dset_destroy(&ds);
	return result;
}

GRAPH_DECL graph_kruskal_f_result_t wfgraph_kruskal_al(const wfgraph_t *g)
{
	pqueue_t          pq;
	wedgef_t         *edges;
	graph_size_t      edge_count;
	graph_size_t      k;
	vertex_id_t       u;
	vertex_id_t       v;
	wedgef_t          e;
	disjoint_set_t    ds;
	graph_kruskal_f_result_t result = {0};

	edges = wfgraph_edges_al(g, &edge_count);

	pqueue_construct(&pq, sizeof(wedgef_t), kruskal_cmpf);
	for (k = 0; k < edge_count; ++k) {
		pqueue_push(&pq, &edges[k]);
	}
	GRAPH_FREE(edges);

	dset_construct(&ds, g->vertex_count);
	result.MST = (wedgef_t *)GRAPH_MALLOC((g->vertex_count - 1) * sizeof(wedgef_t));

	while (!pqueue_empty(&pq)) {
		e = pqueue_top(&pq, wedgef_t); pqueue_pop(&pq);
		u = e.from; v = e.to;
		if (!dset_joined(&ds, u, v)) {
			dset_join(&ds, u, v);
			result.span_weight += e.weight;
			result.MST[result.mst_size++] = e;
			if (result.mst_size == g->vertex_count - 1) break;
		}
	}

	pqueue_destroy(&pq);
	dset_destroy(&ds);
	return result;
}

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