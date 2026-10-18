#ifndef TSP_H
#define TSP_H

/* d is a flat n*n symmetric distance matrix. A tour is a permutation of 0..n-1 that
   starts at tour[0] and returns there at the end. */
int  tour_cost(const int *d, int n, const int *tour);
void tsp_nearest_neighbor(const int *d, int n, int start, int *tour);
int  tsp_two_opt(const int *d, int n, int *tour);        /* improves in place, returns passes */
int  tsp_brute_force(const int *d, int n, int *best);    /* exact, start fixed at 0, use n <= 11 */

#endif
