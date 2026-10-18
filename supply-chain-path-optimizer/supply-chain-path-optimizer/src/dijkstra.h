#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include "graph.h"

#define INF 1000000000

/* Baseline: O(V^2) linear scan for the next closest node. */
void dijkstra_array(const Graph *g, int src, int *dist, int *prev);
/* Optimized: binary min-heap, O((V+E) log V). */
void dijkstra_heap(const Graph *g, int src, int *dist, int *prev);
/* Rebuilds src->dst path from prev[]; writes node ids to out, returns node count (0 if unreachable). */
int  path_build(const int *prev, int src, int dst, int *out);

#endif
