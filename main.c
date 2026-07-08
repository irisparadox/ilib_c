#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LINALG_IMPLEMENTATION
#include "linalg.h"

#define GRAPH_IMPLEMENTATION
#include "graph.h"

#define QUEUE_IMPLEMENTATION
#define PQUEUE_IMPLEMENTATION
#include "queue.h"

#define ALLOCAI_IMPLEMENTATION
#include "allocai.h"

void vectest()
{
	vec2i_t a = { 10, -5 };
	vec2i_t b = { 99, 42 };

	vmask2_t m = v2cmpgt_vv(a, b);

	vec2i_t r1 = v2select(a, b, m);
	
	printf("x: %d, y: %d\n", r1.x, r1.y);

	vec2f_t af = { 99.15, -5.0 };
	vec2f_t bf = { 99.10, 42.590};

	vmask2_t mf = vf2cmplt_vv(af, bf);

	vec2f_t r2 = vf2select(af, bf, mf);

	printf("x: %.3f, y: %.3f\n", r2.x, r2.y);
	printf("%.3f\n", vf2redmin_v(r2));
	printf("%.3f %.3f\n", vf2length_v(r2), vf2lengthsq_v(r2));

	vec2f_t norm = vf2normalize_v(r2);
	printf("x: %.3f, y: %.3f\n", norm.x, norm.y);

	vec3i_t v1 = { 1, 1, 1 };
	vec3i_t v2 = { -1, 1, 0 };

	vec3i_t cross = v3cross_vv(v1, v2);
	printf("x: %df, y: %df, z: %df\n", cross.x, cross.y, cross.z);
}

void graphtest_undirected(void)
{
	/* Example graph (0-indexed, undirected):
	 *   0 -- 1
	 *   2 -- 3
	 *   3 -- 0
	 * vertex_count = 4, edge_count = 3
	 */
	edge_t edges[] = {
		{0, 1},
		{2, 3},
		{3, 0},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	graph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	graph_construct(&g, edges, vertex_count, edge_count);
	result = graph_dfs_al(&g);

	printf("[graph_t] component_count: %u\n", result.component_count);
	printf("[graph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[graph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[graph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	graph_destroy(&g);
}

void graphtest_directed(void)
{
	/* Same shape, directed:
	 *   0 -> 1
	 *   2 -> 3
	 *   3 -> 0
	 */
	edge_t edges[] = {
		{0, 1},
		{2, 3},
		{3, 0},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	dgraph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	dgraph_construct(&g, edges, vertex_count, edge_count);
	result = dgraph_dfs_al(&g);

	printf("[dgraph_t] component_count: %u\n", result.component_count);
	printf("[dgraph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[dgraph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[dgraph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	dgraph_destroy(&g);
}

void graphtest_weighted_int(void)
{
	wedgei_t edges[] = {
		{0, 1, 5},
		{2, 3, 7},
		{3, 0, 2},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	wigraph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	wigraph_construct(&g, edges, vertex_count, edge_count);
	result = wigraph_dfs_al(&g);

	printf("[wigraph_t] component_count: %u\n", result.component_count);
	printf("[wigraph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[wigraph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[wigraph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	wigraph_destroy(&g);
}

void graphtest_weighted_float(void)
{
	wedgef_t edges[] = {
		{0, 1, 5.0f},
		{2, 3, 7.0f},
		{3, 0, 2.0f},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	wfgraph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	wfgraph_construct(&g, edges, vertex_count, edge_count);
	result = wfgraph_dfs_al(&g);

	printf("[wfgraph_t] component_count: %u\n", result.component_count);
	printf("[wfgraph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[wfgraph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[wfgraph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	wfgraph_destroy(&g);
}

void graphtest_weighted_int_directed(void)
{
	wedgei_t edges[] = {
		{0, 1, 5},
		{2, 3, 7},
		{3, 0, 2},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	widgraph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	widgraph_construct(&g, edges, vertex_count, edge_count);
	result = widgraph_dfs_al(&g);

	printf("[widgraph_t] component_count: %u\n", result.component_count);
	printf("[widgraph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[widgraph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[widgraph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	widgraph_destroy(&g);
}

void graphtest_weighted_float_directed(void)
{
	wedgef_t edges[] = {
		{0, 1, 5.0f},
		{2, 3, 7.0f},
		{3, 0, 2.0f},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	wfdgraph_t g;
	graph_dfs_result_t result;
	graph_size_t i;

	wfdgraph_construct(&g, edges, vertex_count, edge_count);
	result = wfdgraph_dfs_al(&g);

	printf("[wfdgraph_t] component_count: %u\n", result.component_count);
	printf("[wfdgraph_t] has_cycle: %s\n", result.has_cycle ? "yes" : "no");
	printf("[wfdgraph_t] component_id: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.component_id[i]);
	printf("\n[wfdgraph_t] postorder: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.postorder[i]);
	printf("\n");

	free(result.component_id);
	free(result.postorder);
	wfdgraph_destroy(&g);
}

void graphtest(void)
{
	graphtest_undirected();
	graphtest_directed();
	graphtest_weighted_int();
	graphtest_weighted_float();
	graphtest_weighted_int_directed();
	graphtest_weighted_float_directed();
}

int queuetest(void)
{
	queue_t q;
	if (queue_construct(&q, sizeof(int)) != 0) {
		printf("Error on queue_construct(): %u\n", que_err);
		return que_err;
	}

	int ret = queue_push(&q, &(int){42});

	printf("%d\n", queue_front(&q, int));

	ret = queue_push(&q, &(int){35});
	ret = queue_pop(&q);

	printf("%d\n", queue_front(&q, int));

	ret = queue_pop(&q);

	if (queue_empty(&q)) {
		printf("pop returned: %d, front is: NULL\n", ret);
	} else {
		printf("pop returned: %d, front is: %d\n", ret, queue_front(&q, int));
	}
	
	if (queue_destroy(&q) != 0) {
		printf("Error on queue_destroy(): %u\n", que_err);
		return que_err;
	}

	return ret;
}

void bfstest_undirected(void)
{
	/* Example graph (0-indexed, undirected):
	 *   0 -- 1
	 *   1 -- 2
	 *   2 -- 3
	 *   0 -- 3
	 * vertex_count = 4, edge_count = 4
	 * Expect BFS from origin 0: dist = [0, 1, 2, 1], prev[1]=0, prev[3]=0, prev[2]=1 or 3
	 */
	edge_t edges[] = {
		{0, 1},
		{1, 2},
		{2, 3},
		{0, 3},
	};
	graph_size_t edge_count   = 4;
	graph_size_t vertex_count = 4;
	graph_t g;
	graph_bfs_result_t result;
	graph_size_t i;

	graph_construct(&g, edges, vertex_count, edge_count);
	result = graph_bfs_al(&g, 0);

	printf("[graph_t BFS] origin: %u\n", result.origin);

	printf("visited: ");
	for (i = 0; i < vertex_count; i++) printf("%d ", result.visited[i]);
	printf("\n");

	printf("dist: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.dist[i]);
	printf("\n");

	printf("prev: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.prev[i]);
	printf("\n");

	GRAPH_FREE(result.visited);
	GRAPH_FREE(result.prev);
	GRAPH_FREE(result.dist);
	graph_destroy(&g);
}

void bfstest_directed(void)
{
	/* Directed:
	 *   0 -> 1
	 *   1 -> 2
	 *   2 -> 3
	 * vertex_count = 4, edge_count = 3
	 * Expect BFS from origin 0: dist = [0, 1, 2, 3]
	 */
	edge_t edges[] = {
		{0, 1},
		{1, 2},
		{2, 3},
	};
	graph_size_t edge_count   = 3;
	graph_size_t vertex_count = 4;
	dgraph_t g;
	graph_bfs_result_t result;
	graph_size_t i;

	dgraph_construct(&g, edges, vertex_count, edge_count);
	result = dgraph_bfs_al(&g, 0);

	printf("[dgraph_t BFS] origin: %u\n", result.origin);

	printf("visited: ");
	for (i = 0; i < vertex_count; i++) printf("%d ", result.visited[i]);
	printf("\n");

	printf("dist: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.dist[i]);
	printf("\n");

	printf("prev: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", result.prev[i]);
	printf("\n");

	GRAPH_FREE(result.visited);
	GRAPH_FREE(result.prev);
	GRAPH_FREE(result.dist);
	dgraph_destroy(&g);
}

void bfstest(void)
{
	bfstest_undirected();
	bfstest_directed();
}

PQ_DEFINE_CMP_DESC(cmp, int)

int pquetest(void)
{
	pqueue_t pq;
	
	if (pqueue_construct(&pq, sizeof(int), cmp) != 0) {
		printf("Error on pqueue_construct(): %u\n", que_err);
		return que_err;
	}

	pqueue_push(&pq, &(int){42});
	pqueue_push(&pq, &(int){32});
	pqueue_push(&pq, &(int){30});

	pqueue_pop(&pq);

	printf("pq top: %d\n", pqueue_top(&pq, int));

	if (pqueue_destroy(&pq) != 0) {
		printf("Error on pqueue_destroy(): %u\n", que_err);
		return que_err;
	}

	return 0;
}

void dijkstratest(void)
{
	/* Weighted undirected graph (0-indexed):
	 *   0 -1(w4)- 1
	 *   0 -1(w1)- 2
	 *   2 -1(w2)- 1
	 *   1 -1(w1)- 3
	 *   2 -1(w5)- 3
	 *   3 -1(w3)- 4
	 * vertex_count = 5, edge_count = 6
	 *
	 * Expected shortest paths from origin 0:
	 *   dist[0] = 0
	 *   dist[2] = 1   (0->2)
	 *   dist[1] = 3   (0->2->1, 1+2=3, beats direct 0->1=4)
	 *   dist[3] = 4   (0->2->1->3, 3+1=4)
	 *   dist[4] = 7   (0->2->1->3->4, 4+3=7)
	 */
	wedgei_t edges_i[] = {
		{0, 1, 4},
		{0, 2, 1},
		{2, 1, 2},
		{1, 3, 1},
		{2, 3, 5},
		{3, 4, 3},
	};
	wedgef_t edges_f[] = {
		{0, 1, 4.0f},
		{0, 2, 1.0f},
		{2, 1, 2.0f},
		{1, 3, 1.0f},
		{2, 3, 5.0f},
		{3, 4, 3.0f},
	};
	graph_size_t edge_count   = 6;
	graph_size_t vertex_count = 5;

	wigraph_t gi;
	wfgraph_t gf;
	graph_dijkstra_i_result_t ri;
	graph_dijkstra_f_result_t rf;
	graph_size_t i;

	wigraph_construct(&gi, edges_i, vertex_count, edge_count);
	ri = wigraph_dijkstra_al(&gi, 0);

	printf("[wigraph_t Dijkstra] origin: %u\n", ri.origin);
	printf("dist: ");
	for (i = 0; i < vertex_count; i++) printf("%d ", ri.dist[i]);
	printf("\nprev: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", ri.prev[i]);
	printf("\n");

	free(ri.dist);
	free(ri.prev);
	free(ri.visited);
	wigraph_destroy(&gi);

	wfgraph_construct(&gf, edges_f, vertex_count, edge_count);
	rf = wfgraph_dijkstra_al(&gf, 0);

	printf("[wfgraph_t Dijkstra] origin: %u\n", rf.origin);
	printf("dist: ");
	for (i = 0; i < vertex_count; i++) printf("%.2f ", rf.dist[i]);
	printf("\nprev: ");
	for (i = 0; i < vertex_count; i++) printf("%u ", rf.prev[i]);
	printf("\n");

	free(rf.dist);
	free(rf.prev);
	free(rf.visited);
	wfgraph_destroy(&gf);
}

void kruskaltest(void)
{
	/* Expected MST (Kruskal, sorted by weight):
	 *   (0,2,w1) join {0,2}
	 *   (1,3,w1) join {1,3}
	 *   (2,1,w2) join {0,2,1,3}
	 *   (3,4,w3) join {0,1,2,3,4}  <- span_size == vertex_count - 1, stop
	 * Expected span_weight = 1 + 1 + 2 + 3 = 7
	 */
	wedgei_t edges_i[] = {
		{0, 1, 4},
		{0, 2, 1},
		{2, 1, 2},
		{1, 3, 1},
		{2, 3, 5},
		{3, 4, 3},
	};

	/* Expected MST (Kruskal, sorted by weight):
	*   (0,2,w1.2) join {0,2}
	*   (1,3,w1.8) join {1,3}
	*   (2,1,w2.4) join {0,2,1,3}
	*   (3,4,w3.1) join {0,1,2,3,4}
	* Expected span_weight = 1.2 + 1.8 + 2.4 + 3.1 = 8.5
	*/
	wedgef_t edges_f[] = {
		{0, 1, 4.7f},
		{0, 2, 1.2f},
		{2, 1, 2.4f},
		{1, 3, 1.8f},
		{2, 3, 5.6f},
		{3, 4, 3.1f},
	};

	graph_size_t edge_count   = 6;
	graph_size_t vertex_count = 5;

	wigraph_t gi;
	wfgraph_t gf;

	graph_kruskal_i_result_t ri;
	graph_kruskal_f_result_t rf;

	graph_size_t i;

	wigraph_construct(&gi, edges_i, vertex_count, edge_count);
	wfgraph_construct(&gf, edges_f, vertex_count, edge_count);

	ri = wigraph_kruskal_al(&gi);
	rf = wfgraph_kruskal_al(&gf);

	printf("[wigraph_t Kruskal] span_weight: %d (expected 7)\n",
		ri.span_weight);
	printf("[wigraph_t Kruskal] mst:");
	for (i = 0; i < ri.mst_size; ++i) printf(" [%d->%d, %d]", ri.MST[i].from, ri.MST[i].to, ri.MST[i].weight);
	printf("\n");
	printf("[wfgraph_t Kruskal] span_weight: %.1f (expected 8.5)\n",
		rf.span_weight);
	printf("[wfgraph_t Kruskal] mst:");
	for (i = 0; i < rf.mst_size; ++i) printf(" [%d->%d, %.1f]", rf.MST[i].from, rf.MST[i].to, rf.MST[i].weight);
	printf("\n");

	free(ri.MST);
	free(rf.MST);
	wigraph_destroy(&gi);
	wfgraph_destroy(&gf);
}

static void
test_floyd_directed_int(void)
{
	widgraph_t g;
	wedgei_t edges[] = {
		{ .from = 0, .to = 1, .weight = 5 },
		{ .from = 0, .to = 3, .weight = 10 },
		{ .from = 1, .to = 2, .weight = 3 },
		{ .from = 2, .to = 3, .weight = 1 },
	};
	graph_floyd_i_result_t r;
	graph_size_t n = 4;
	vertex_id_t *path;
	graph_size_t path_len;

	widgraph_construct(&g, edges, n, 4);
	r = widgraph_floyd_al(&g);

	assert(r.has_negative_cycle == 0);
	assert(r.dist[0 * n + 3] == 9);   /* 0->1->2->3 beats direct 0->3 */
	assert(r.dist[0 * n + 1] == 5);
	assert(r.dist[0 * n + 2] == 8);
	assert(r.dist[1 * n + 3] == 4);
	assert(r.dist[3 * n + 0] == I_DIJKSTRA_INT_INF); /* unreachable, no back edges */

	path = graph_floyd_path_al(r.next, n, 0, 3, &path_len);
	assert(path != NULL);
	assert(path_len == 4);
	assert(path[0] == 0 && path[1] == 1 && path[2] == 2 && path[3] == 3);
	free(path);

	path = graph_floyd_path_al(r.next, n, 3, 0, &path_len);
	assert(path == NULL);
	assert(path_len == 0);

	free(r.dist);
	free(r.next);
	widgraph_destroy(&g);

	printf("test_floyd_directed_int: OK\n");
}

static void
test_floyd_negative_cycle_int(void)
{
	widgraph_t g;
	wedgei_t edges[] = {
		{ .from = 0, .to = 1, .weight = 1 },
		{ .from = 1, .to = 0, .weight = -2 },
	};
	graph_floyd_i_result_t r;

	widgraph_construct(&g, edges, 2, 2);
	r = widgraph_floyd_al(&g);

	assert(r.has_negative_cycle == 1);

	free(r.dist);
	free(r.next);
	widgraph_destroy(&g);

	printf("test_floyd_negative_cycle_int: OK\n");
}

static void
test_floyd_undirected_float(void)
{
	wfgraph_t g;
	wedgef_t edges[] = {
		{ .from = 0, .to = 1, .weight = 2.0f },
		{ .from = 1, .to = 2, .weight = 4.0f },
		{ .from = 0, .to = 2, .weight = 9.0f },
	};
	graph_floyd_f_result_t r;
	graph_size_t n = 3;
	vertex_id_t *path;
	graph_size_t path_len;

	wfgraph_construct(&g, edges, n, 3);
	r = wfgraph_floyd_al(&g);

	assert(r.has_negative_cycle == 0);
	assert(r.dist[0 * n + 2] == 6.0f);            /* 0-1-2 beats direct 0-2 */
	assert(r.dist[0 * n + 2] == r.dist[2 * n + 0]); /* symmetric */
	assert(r.dist[1 * n + 1] == 0.0f);

	path = graph_floyd_path_al(r.next, n, 0, 2, &path_len);
	assert(path != NULL);
	assert(path_len == 3);
	assert(path[0] == 0 && path[1] == 1 && path[2] == 2);
	free(path);

	free(r.dist);
	free(r.next);
	wfgraph_destroy(&g);

	printf("test_floyd_undirected_float: OK\n");
}

void test_floyd(void)
{
	test_floyd_directed_int();
	test_floyd_negative_cycle_int();
	test_floyd_undirected_float();
}

void test_arena_alloc(void)
{
	al_arena_t a;
	al_byte_t *p1, *p2, *p3;
	int i;

	/* --- init: invalid arg --- */
	a = al_arena_init(0);
	assert(a.base == NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("init_invalid_arg: OK\n");

	/* --- init: normal case --- */
	a = al_arena_init(64);
	assert(a.base != NULL);
	assert(a.offset == 0);
	assert(a.capacity == 64);
	assert(al_errno == AL_OK);
	printf("init_normal_case: OK\n");

	/* --- alloc: invalid args --- */
	p1 = al_arena_alloc(NULL, 8);
	assert(p1 == NULL);
	assert(al_errno == AL_ERRINVAL);

	p1 = al_arena_alloc(&a, 0);
	assert(p1 == NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("alloc_invalid_args: OK\n");

	/* --- alloc: normal case, alignment --- */
	p1 = al_arena_alloc(&a, 1);
	assert(p1 != NULL);
	assert(al_errno == AL_OK);
	assert(a.offset == 1);

	p2 = al_arena_alloc(&a, 1);
	assert(p2 != NULL);
	assert(p2 == p1 + ALLOC_ALIGNMENT);
	printf("alloc_normal_case_alignment: OK\n");

	/* --- alloc: exhaustion --- */
	p3 = al_arena_alloc(&a, 1000); /* way bigger than remaining capacity */
	assert(p3 == NULL);
	assert(al_errno == AL_ERRNOMEM);
	printf("alloc_exhaustion: OK\n");

	/* --- calloc: zero-init check --- */
	al_arena_reset(&a);
	assert(a.offset == 0);
	assert(al_errno == AL_OK);

	p1 = al_arena_calloc(&a, 4, sizeof(int));
	assert(p1 != NULL);
	for (i = 0; i < (int)(4 * sizeof(int)); ++i) {
		assert(p1[i] == 0);
	}
	printf("calloc_zero_init_check: OK\n");

	/* --- calloc: overflow guard --- */
	p2 = al_arena_calloc(&a, AL_SIZE_MAX, 2);
	assert(p2 == NULL);
	assert(al_errno == AL_ERRNOMEM);
	printf("calloc_overflow_guard: OK\n");

	/* --- reset: invalid arg --- */
	al_arena_reset(NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("reset_invalid_arg: OK\n");

	/* --- free: normal case --- */
	al_arena_free(&a);
	assert(a.base == NULL);
	assert(a.offset == 0);
	assert(a.capacity == 0);
	assert(al_errno == AL_OK);
	printf("free_normal_case: OK\n");

	/* --- free: double-free is a safe no-op --- */
	al_arena_free(&a);
	assert(al_errno == AL_OK);
	printf("free_double_free_safe_noop: OK\n");

	/* --- free: invalid arg --- */
	al_arena_free(NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("free_invalid_arg: OK\n");

	printf("all alloc.h tests passed\n");
}

void test_stack_alloc(void)
{
	al_stack_t s;
	al_byte_t *p1, *p2;
	al_size_t saved;
	int i;

	/* --- init: invalid arg --- */
	s = al_stack_init(0);
	assert(s.arena.base == NULL);
	assert(s.top == 0);
	assert(al_errno == AL_ERRINVAL);
	printf("init_invalid_arg: OK\n");

	/* --- init: normal case --- */
	s = al_stack_init(64);
	assert(s.arena.base != NULL);
	assert(s.arena.offset == 0);
	assert(s.arena.capacity == 64);
	assert(s.top == 0);
	assert(al_errno == AL_OK);
	printf("init_normal_case: OK\n");

	/* --- alloc: invalid args --- */
	p1 = al_stack_alloc(NULL, 8);
	assert(p1 == NULL);
	assert(al_errno == AL_ERRINVAL);

	p1 = al_stack_alloc(&s, 0);
	assert(p1 == NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("alloc_invalid_args: OK\n");

	/* --- alloc: normal case --- */
	p1 = al_stack_alloc(&s, 8);
	assert(p1 != NULL);
	assert(al_errno == AL_OK);
	assert(s.arena.offset == 8);
	printf("alloc_normal_case: OK\n");

	/* --- push: invalid arg --- */
	al_stack_push(NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("push_invalid_arg: OK\n");

	/* --- push/pop: restore allocation mark --- */
	saved = s.arena.offset;

	al_stack_push(&s);
	assert(s.top == 1);
	assert(s.marks[0] == saved);
	assert(al_errno == AL_OK);

	p2 = al_stack_alloc(&s, 16);
	assert(p2 != NULL);
	assert(s.arena.offset > saved);

	al_stack_pop(&s);
	assert(s.top == 0);
	assert(s.arena.offset == saved);
	assert(al_errno == AL_OK);
	printf("push_pop_restore_mark: OK\n");

	/* --- pop: invalid arg --- */
	al_stack_pop(NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("pop_invalid_arg: OK\n");

	/* --- pop: underflow --- */
	al_stack_pop(&s);
	assert(al_errno == AL_ERRSTKUF);
	printf("pop_underflow: OK\n");

	/* --- push: overflow --- */
	for (i = 0; i < AL_STACK_MAX_MARKS; ++i)
		al_stack_push(&s);

	assert(s.top == AL_STACK_MAX_MARKS);
	assert(al_errno == AL_OK);

	al_stack_push(&s);
	assert(al_errno == AL_ERRSTKOF);
	assert(s.top == AL_STACK_MAX_MARKS);
	printf("push_overflow: OK\n");

	/* --- free: normal case --- */
	al_stack_free(&s);
	assert(s.arena.base == NULL);
	assert(s.arena.offset == 0);
	assert(s.arena.capacity == 0);
	assert(s.top == 0);
	assert(al_errno == AL_OK);
	printf("free_normal_case: OK\n");

	/* --- free: double-free is a safe no-op --- */
	al_stack_free(&s);
	assert(al_errno == AL_OK);
	printf("free_double_free_safe_noop: OK\n");

	/* --- free: invalid arg --- */
	al_stack_free(NULL);
	assert(al_errno == AL_ERRINVAL);
	printf("free_invalid_arg: OK\n");

	printf("all stack tests passed\n");
}

int main(void)
{
	test_stack_alloc();
	return 0;
}