#include <stdlib.h>
#include "dijkstra.h"

void dijkstra_array(const Graph *g, int src, int *dist, int *prev) {
    int n = g->n;
    char *done = calloc((size_t)n, 1);
    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;
    for (int it = 0; it < n; it++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!done[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        done[u] = 1;
        for (Edge *e = g->adj[u]; e; e = e->next)
            if (dist[u] + e->w < dist[e->to]) { dist[e->to] = dist[u] + e->w; prev[e->to] = u; }
    }
    free(done);
}

/* ---- binary min-heap of (distance, node) with lazy deletion ---- */
typedef struct { int d, v; } HeapItem;
typedef struct { HeapItem *a; int size, cap; } Heap;

static void heap_push(Heap *h, int d, int v) {
    if (h->size == h->cap) {
        h->cap *= 2;
        h->a = realloc(h->a, (size_t)h->cap * sizeof(HeapItem));
    }
    int i = h->size++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->a[p].d <= d) break;
        h->a[i] = h->a[p];
        i = p;
    }
    h->a[i].d = d; h->a[i].v = v;
}

static HeapItem heap_pop(Heap *h) {
    HeapItem top = h->a[0], last = h->a[--h->size];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= h->size) break;
        if (c + 1 < h->size && h->a[c + 1].d < h->a[c].d) c++;
        if (h->a[c].d >= last.d) break;
        h->a[i] = h->a[c];
        i = c;
    }
    if (h->size > 0) h->a[i] = last;
    return top;
}

void dijkstra_heap(const Graph *g, int src, int *dist, int *prev) {
    for (int i = 0; i < g->n; i++) { dist[i] = INF; prev[i] = -1; }
    Heap h = { malloc(64 * sizeof(HeapItem)), 0, 64 };
    dist[src] = 0;
    heap_push(&h, 0, src);
    while (h.size > 0) {
        HeapItem it = heap_pop(&h);
        if (it.d > dist[it.v]) continue;              /* stale entry */
        for (Edge *e = g->adj[it.v]; e; e = e->next) {
            int nd = it.d + e->w;
            if (nd < dist[e->to]) { dist[e->to] = nd; prev[e->to] = it.v; heap_push(&h, nd, e->to); }
        }
    }
    free(h.a);
}

int path_build(const int *prev, int src, int dst, int *out) {
    int len = 0;
    for (int v = dst; v != -1; v = prev[v]) {
        out[len++] = v;
        if (v == src) break;
    }
    if (len == 0 || out[len - 1] != src) return 0;
    for (int i = 0, j = len - 1; i < j; i++, j--) { int t = out[i]; out[i] = out[j]; out[j] = t; }
    return len;
}
