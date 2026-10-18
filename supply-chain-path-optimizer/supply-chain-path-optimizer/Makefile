CC     ?= gcc
CFLAGS  = -std=c99 -O2 -Wall -Wextra
SRC     = src/main.c src/graph.c src/dijkstra.c src/tsp.c src/supply.c

supply_chain_optimizer: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

run: supply_chain_optimizer
	./supply_chain_optimizer

clean:
	rm -f supply_chain_optimizer supply_chain_optimizer.exe
