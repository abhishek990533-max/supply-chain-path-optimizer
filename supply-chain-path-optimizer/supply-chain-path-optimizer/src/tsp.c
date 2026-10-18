#include <limits.h>
#include <string.h>
#include "tsp.h"

#define D(i, j) d[(i) * n + (j)]

int tour_cost(const int *d, int n, const int *tour) {
    int c = 0;
    for (int i = 0; i < n; i++) c += D(tour[i], tour[(i + 1) % n]);
    return c;
}

void tsp_nearest_neighbor(const int *d, int n, int start, int *tour) {
    char used[64] = {0};
    tour[0] = start; used[start] = 1;
    for (int k = 1; k < n; k++) {
        int cur = tour[k - 1], best = -1;
        for (int v = 0; v < n; v++)
            if (!used[v] && (best == -1 || D(cur, v) < D(cur, best))) best = v;
        tour[k] = best; used[best] = 1;
    }
}

int tsp_two_opt(const int *d, int n, int *tour) {
    int improved = 1, passes = 0;
    while (improved) {
        improved = 0; passes++;
        for (int i = 1; i < n - 1; i++)
            for (int j = i + 1; j < n; j++) {
                int a = tour[i - 1], b = tour[i], c = tour[j], e = tour[(j + 1) % n];
                if (D(a, c) + D(b, e) < D(a, b) + D(c, e)) {      /* swapping two edges shortens the tour */
                    for (int l = i, r = j; l < r; l++, r--) { int t = tour[l]; tour[l] = tour[r]; tour[r] = t; }
                    improved = 1;
                }
            }
    }
    return passes;
}

static void bf(const int *d, int n, int *cur, char *used, int depth, int cost, int *best, int *best_tour) {
    if (cost >= *best) return;                                    /* prune */
    if (depth == n) {
        int total = cost + D(cur[n - 1], cur[0]);
        if (total < *best) { *best = total; memcpy(best_tour, cur, (size_t)n * sizeof(int)); }
        return;
    }
    for (int v = 1; v < n; v++)
        if (!used[v]) {
            used[v] = 1; cur[depth] = v;
            bf(d, n, cur, used, depth + 1, cost + D(cur[depth - 1], v), best, best_tour);
            used[v] = 0;
        }
}

int tsp_brute_force(const int *d, int n, int *best_tour) {
    int cur[64] = {0}, best = INT_MAX;
    char used[64] = {0};
    used[0] = 1;
    bf(d, n, cur, used, 1, 0, &best, best_tour);
    return best;
}
