#ifndef GRAPH_H
#define GRAPH_H

/* Small deterministic RNG (xorshift32) so results are identical on every OS. */
typedef struct { unsigned s; } Rng;
static inline unsigned rng_u32(Rng *r) {
    unsigned x = r->s ? r->s : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    r->s = x;
    return x;
}
static inline int rng_range(Rng *r, int lo, int hi) {
    return lo + (int)(rng_u32(r) % (unsigned)(hi - lo + 1));
}

/* Undirected weighted graph stored as adjacency lists. */
typedef struct Edge { int to, w; struct Edge *next; } Edge;
typedef struct { int n, m; Edge **adj; } Graph;

Graph *graph_create(int n);
void   graph_add_edge(Graph *g, int u, int v, int w);
void   graph_free(Graph *g);
/* Connected random road network: random tree + extra_per_node*n extra roads (1-100 km). */
Graph *graph_random(int n, int extra_per_node, Rng *rng);

#endif
