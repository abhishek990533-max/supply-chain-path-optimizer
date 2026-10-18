#include <stdlib.h>
#include "graph.h"

Graph *graph_create(int n) {
    Graph *g = malloc(sizeof *g);
    g->n = n;
    g->m = 0;
    g->adj = calloc((size_t)n, sizeof(Edge *));
    return g;
}

static void add_directed(Graph *g, int u, int v, int w) {
    Edge *e = malloc(sizeof *e);
    e->to = v; e->w = w; e->next = g->adj[u];
    g->adj[u] = e;
}

void graph_add_edge(Graph *g, int u, int v, int w) {
    add_directed(g, u, v, w);
    add_directed(g, v, u, w);
    g->m++;
}

void graph_free(Graph *g) {
    for (int i = 0; i < g->n; i++) {
        Edge *e = g->adj[i];
        while (e) { Edge *nx = e->next; free(e); e = nx; }
    }
    free(g->adj);
    free(g);
}

Graph *graph_random(int n, int extra_per_node, Rng *rng) {
    Graph *g = graph_create(n);
    for (int i = 1; i < n; i++)                       /* guarantees connectivity */
        graph_add_edge(g, i, rng_range(rng, 0, i - 1), rng_range(rng, 1, 100));
    for (int k = 0; k < n * extra_per_node; k++) {    /* extra roads */
        int u = rng_range(rng, 0, n - 1), v = rng_range(rng, 0, n - 1);
        if (u != v) graph_add_edge(g, u, v, rng_range(rng, 1, 100));
    }
    return g;
}
