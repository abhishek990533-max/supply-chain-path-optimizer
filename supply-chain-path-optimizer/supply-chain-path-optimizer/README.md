# Supply Chain Path Optimizer

A C project that plans routes on a 200+ node road network and simulates the supply chain around it. It uses **Dijkstra's algorithm** for shortest paths, **TSP heuristics** (nearest neighbour + 2-opt) for delivery tours, and hand-built data structures for inventory and supplier tracking.

> The road network and demand are randomly generated (fixed seed, so runs are repeatable). No external libraries needed.

## What it does

| Module | File | Data structure / algorithm |
|---|---|---|
| Road network | `graph.c` | Weighted undirected graph, adjacency lists |
| Shortest paths | `dijkstra.c` | Dijkstra with a **binary min-heap** (`O((V+E) log V)`) vs a plain **O(V²) array** baseline |
| Delivery tours | `tsp.c` | Nearest-neighbour + **2-opt** local search, checked against an exact branch-and-bound solver |
| Inventory | `supply.c` | **Hash table** (chaining) keyed by SKU, reorder-level detection |
| Suppliers + transport | `supply.c` | Picks the supplier with the lowest landed cost (goods + transport km from Dijkstra) |
| Demo / benchmark | `main.c` | Runs everything and prints results |

## Build and run

```bash
gcc -std=c99 -O2 -o supply_chain_optimizer src/*.c
./supply_chain_optimizer            # default: 250 nodes, seed 42
./supply_chain_optimizer 500 7      # custom: nodes, seed
```
(or `make run` if you have `make`). On Windows with MinGW, run `supply_chain_optimizer.exe`.

## Results (250-node network, sample run)

**Dijkstra: heap vs array** (time per single-source run, 50 sources per size; your numbers will differ by machine)

| Nodes | Array (µs) | Heap (µs) | Speedup |
|---|---|---|---|
| 200 | 70.6 | 23.7 | 3.0x |
| 500 | 308.4 | 67.1 | 4.6x |
| 1000 | 1078.5 | 142.7 | 7.6x |
| 2000 | 3851.3 | 319.8 | 12.0x |
| 4000 | 18475.4 | 982.3 | 18.8x |

Both versions are checked against each other on every source, and distances match.

**Delivery tour quality** (100 random 9-stop instances vs the exact optimum)
- Nearest neighbour: 7.0% longer than optimal on average
- Nearest neighbour + 2-opt: 0.5% longer on average (worst case 5.5%)

**Inventory simulation:** 7 days, 6 SKUs, reorders go to the cheapest landed-cost supplier; all demand was served in the sample run.

Full output: [`sample_output.txt`](sample_output.txt)

## Project layout
```
src/
  graph.h / graph.c        road network + random generator
  dijkstra.h / dijkstra.c  array and heap Dijkstra, path rebuild
  tsp.h / tsp.c            NN, 2-opt, exact solver
  supply.h / supply.c      inventory hash table, supplier selection
  main.c                   benchmark + demos
Makefile
sample_output.txt
```

## Ideas to extend
Add A* search, Christofides or Or-opt, time windows, or load a real road network from a CSV file.
