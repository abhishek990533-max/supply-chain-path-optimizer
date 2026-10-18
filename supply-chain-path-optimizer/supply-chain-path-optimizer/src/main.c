/* Supply Chain Path Optimizer
   Road network (graph) -> shortest paths (Dijkstra) -> delivery tours (TSP heuristics)
   -> inventory / supplier / transport simulation. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "graph.h"
#include "dijkstra.h"
#include "tsp.h"
#include "supply.h"

#define MAXS 12

static double now_us(void) { return (double)clock() * 1e6 / CLOCKS_PER_SEC; }

/* ---------- 1. Dijkstra: O(V^2) array vs binary heap ---------- */
static void benchmark(void) {
    int sizes[] = {200, 500, 1000, 2000, 4000};
    printf("%-7s %-7s %-16s %-16s %-8s\n", "Nodes", "Roads", "Array (us/run)", "Heap (us/run)", "Speedup");
    for (int s = 0; s < 5; s++) {
        int n = sizes[s], nsrc = 50;
        Rng rng = {7u};
        Graph *g = graph_random(n, 2, &rng);
        long reps = 400000000L / ((long)nsrc * n * n);
        if (reps < 1) reps = 1;
        int *da = malloc(n * sizeof(int)), *dh = malloc(n * sizeof(int)), *pv = malloc(n * sizeof(int));

        double t0 = now_us();
        for (long r = 0; r < reps; r++)
            for (int k = 0; k < nsrc; k++) dijkstra_array(g, k * (n / nsrc), da, pv);
        double ta = now_us() - t0;

        t0 = now_us();
        for (long r = 0; r < reps; r++)
            for (int k = 0; k < nsrc; k++) dijkstra_heap(g, k * (n / nsrc), dh, pv);
        double th = now_us() - t0;

        int ok = 1;                                   /* both versions must agree on every distance */
        for (int k = 0; k < nsrc; k++) {
            dijkstra_array(g, k * (n / nsrc), da, pv);
            dijkstra_heap(g, k * (n / nsrc), dh, pv);
            if (memcmp(da, dh, n * sizeof(int)) != 0) ok = 0;
        }
        double runs = (double)reps * nsrc;
        printf("%-7d %-7d %-16.1f %-16.1f %.1fx%s\n", n, g->m, ta / runs, th / runs, ta / th, ok ? "" : "  MISMATCH!");
        free(da); free(dh); free(pv); graph_free(g);
    }
}

/* ---------- 2. Delivery tour: Nearest Neighbour, 2-opt, exact optimum ---------- */
static void solve_instance(const Graph *g, Rng *rng, int ns, int verbose, double *gap_nn, double *gap_2opt) {
    int n = g->n, stops[MAXS], d[MAXS * MAXS], nn[MAXS], two[MAXS], opt[MAXS];
    int *dist = malloc(n * sizeof(int)), *prev = malloc(n * sizeof(int)), *path = malloc(n * sizeof(int));
    stops[0] = 0;                                     /* node 0 = warehouse */
    for (int i = 1; i < ns;) {
        int c = rng_range(rng, 1, n - 1), dup = 0;
        for (int j = 0; j < i; j++) if (stops[j] == c) dup = 1;
        if (!dup) stops[i++] = c;
    }
    for (int i = 0; i < ns; i++) {                    /* shortest road distance between every pair of stops */
        dijkstra_heap(g, stops[i], dist, prev);
        for (int j = 0; j < ns; j++) d[i * ns + j] = dist[stops[j]];
    }
    tsp_nearest_neighbor(d, ns, 0, nn);
    int c_nn = tour_cost(d, ns, nn);
    memcpy(two, nn, sizeof(int) * ns);
    tsp_two_opt(d, ns, two);
    int c_2 = tour_cost(d, ns, two);
    int c_opt = tsp_brute_force(d, ns, opt);
    if (gap_nn) *gap_nn = 100.0 * (c_nn - c_opt) / c_opt;
    if (gap_2opt) *gap_2opt = 100.0 * (c_2 - c_opt) / c_opt;

    if (verbose) {
        printf("Stops (warehouse + %d customers), tour from NN + 2-opt:\n  ", ns - 1);
        for (int k = 0; k < ns; k++) printf("%d -> ", stops[two[k]]);
        printf("%d\n", stops[two[0]]);
        printf("  Nearest neighbour : %d km\n", c_nn);
        printf("  NN + 2-opt        : %d km (%.1f%% above optimal)\n", c_2, 100.0 * (c_2 - c_opt) / c_opt);
        printf("  Optimal (exact)   : %d km (exact search over %d! orderings, pruned)\n", c_opt, ns - 1);
        int src = stops[two[0]], dst = stops[two[1]];
        dijkstra_heap(g, src, dist, prev);
        int len = path_build(prev, src, dst, path);
        printf("  First leg %d -> %d is %d km, road path: ", src, dst, dist[dst]);
        for (int k = 0; k < len; k++) printf("%d%s", path[k], k + 1 < len ? " > " : "\n");
    }
    free(dist); free(prev); free(path);
}

/* ---------- 3. Inventory + suppliers + transport ---------- */
static void supply_demo(const Graph *g, Rng *rng) {
    int n = g->n;
    int *dist = malloc(n * sizeof(int)), *prev = malloc(n * sizeof(int));
    dijkstra_heap(g, 0, dist, prev);                  /* distances from the warehouse (node 0) */

    Supplier sup[3];
    const char *names[3] = {"Supplier A", "Supplier B", "Supplier C"};
    for (int i = 0; i < 3; i++) {
        sup[i].name = names[i];
        sup[i].node = rng_range(rng, 1, n - 1);
        sup[i].unit_cost = rng_range(rng, 20, 60);
        sup[i].per_km = rng_range(rng, 1, 3);
        printf("  %s: node %d, %d km from warehouse, unit cost %d, transport %d/km\n",
               sup[i].name, sup[i].node, dist[sup[i].node], sup[i].unit_cost, sup[i].per_km);
    }

    const char *skus[6] = {"SKU-001", "SKU-002", "SKU-003", "SKU-004", "SKU-005", "SKU-006"};
    const char *pn[6] = {"Steel brackets", "Hydraulic pumps", "Bearings", "Conveyor belts", "Control boards", "Packaging film"};
    Inventory *inv = inv_create(16);
    for (int i = 0; i < 6; i++) inv_add(inv, skus[i], pn[i], 120, 60, 150);

    long spend = 0; int orders = 0, short_units = 0, demand_total = 0, served = 0;
    for (int day = 1; day <= 7; day++) {
        for (int i = 0; i < 6; i++) {                 /* yesterday's orders arrive this morning */
            Item *it = inv_find(inv, skus[i]);
            if (it->on_order) { it->stock += it->reorder_qty; it->on_order = 0; }
        }
        for (int i = 0; i < 6; i++) {                 /* customer demand */
            int want = rng_range(rng, 15, 45), got = inv_take(inv, skus[i], want);
            demand_total += want; served += got; short_units += want - got;
        }
        Item *low[6];
        int k = inv_low_stock(inv, low, 6);
        printf("Day %d: %d low-stock item(s)\n", day, k);
        for (int i = 0; i < k; i++) {                 /* reorder from the cheapest landed-cost supplier */
            long landed;
            int s = supplier_pick(sup, 3, dist, low[i]->reorder_qty, &landed);
            low[i]->on_order = 1; spend += landed; orders++;
            printf("  reorder %s x%d from %s, landed cost %ld\n", low[i]->sku, low[i]->reorder_qty, sup[s].name, landed);
        }
    }
    printf("7-day totals: %d orders, spend %ld, demand served %d/%d units (%d short)\n",
           orders, spend, served, demand_total, short_units);
    inv_free(inv); free(dist); free(prev);
}

int main(int argc, char **argv) {
    int n = argc > 1 ? atoi(argv[1]) : 250;
    unsigned seed = argc > 2 ? (unsigned)atoi(argv[2]) : 42u;
    if (n < 20) { fprintf(stderr, "Use at least 20 nodes.\n"); return 1; }
    Rng rng = {seed};
    Graph *g = graph_random(n, 2, &rng);

    printf("=== Supply Chain Path Optimizer ===\nNetwork: %d nodes, %d roads (seed %u)\n", g->n, g->m, seed);

    int *dist = malloc(n * sizeof(int)), *prev = malloc(n * sizeof(int));
    dijkstra_heap(g, 0, dist, prev);
    int far = 0, reachable = 0;
    for (int i = 0; i < n; i++) { if (dist[i] < INF) reachable++; if (dist[i] > dist[far]) far = i; }
    printf("From warehouse (node 0): %d/%d nodes reachable, farthest is node %d at %d km\n\n", reachable, n, far, dist[far]);
    free(dist); free(prev);

    printf("--- 1. Dijkstra: O(V^2) array vs binary heap (50 sources per size) ---\n");
    benchmark();

    printf("\n--- 2. Delivery tour (TSP heuristics) ---\n");
    solve_instance(g, &rng, 10, 1, NULL, NULL);
    double sum_nn = 0, sum_2 = 0, worst_2 = 0;
    int trials = 100;
    for (int t = 0; t < trials; t++) {
        double a, b;
        solve_instance(g, &rng, 9, 0, &a, &b);
        sum_nn += a; sum_2 += b; if (b > worst_2) worst_2 = b;
    }
    printf("Quality over %d random 9-stop instances (vs exact optimum):\n", trials);
    printf("  Nearest neighbour avg gap: %.1f%%\n  NN + 2-opt avg gap       : %.1f%% (worst %.1f%%)\n", sum_nn / trials, sum_2 / trials, worst_2);

    printf("\n--- 3. Inventory, suppliers and transport (7-day simulation) ---\n");
    supply_demo(g, &rng);

    graph_free(g);
    return 0;
}
