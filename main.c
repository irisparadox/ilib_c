#include <stdio.h>
#include <stdlib.h>

#define LINALG_IMPLEMENTATION
#include "linalg.h"

#define GRAPH_IMPLEMENTATION
#include "graph.h"

#define QUEUE_IMPLEMENTATION
#include "queue.h"

int que_err = 0;

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

	int ret = queue_push(&q, int, 42);

	printf("%d\n", queue_front(&q, int));

	ret = queue_push(&q, int, 35);
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

int main(void)
{
	bfstest();
	return 0;
}