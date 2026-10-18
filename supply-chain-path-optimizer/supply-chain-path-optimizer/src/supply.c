#include <stdlib.h>
#include <string.h>
#include "supply.h"

static unsigned hash_sku(const char *s) {
    unsigned h = 5381;
    while (*s) h = h * 33u + (unsigned char)*s++;
    return h;
}

Inventory *inv_create(int size) {
    Inventory *inv = malloc(sizeof *inv);
    inv->size = size; inv->count = 0;
    inv->buckets = calloc((size_t)size, sizeof(Item *));
    return inv;
}

Item *inv_add(Inventory *inv, const char *sku, const char *name, int stock, int reorder_level, int reorder_qty) {
    Item *it = calloc(1, sizeof *it);
    strncpy(it->sku, sku, sizeof it->sku - 1);
    strncpy(it->name, name, sizeof it->name - 1);
    it->stock = stock; it->reorder_level = reorder_level; it->reorder_qty = reorder_qty;
    unsigned b = hash_sku(sku) % (unsigned)inv->size;
    it->next = inv->buckets[b];
    inv->buckets[b] = it;
    inv->count++;
    return it;
}

Item *inv_find(const Inventory *inv, const char *sku) {
    for (Item *it = inv->buckets[hash_sku(sku) % (unsigned)inv->size]; it; it = it->next)
        if (strcmp(it->sku, sku) == 0) return it;
    return NULL;
}

int inv_take(Inventory *inv, const char *sku, int qty) {
    Item *it = inv_find(inv, sku);
    if (!it) return 0;
    int got = qty < it->stock ? qty : it->stock;
    it->stock -= got;
    return got;
}

int inv_low_stock(const Inventory *inv, Item **out, int max) {
    int k = 0;
    for (int b = 0; b < inv->size; b++)
        for (Item *it = inv->buckets[b]; it && k < max; it = it->next)
            if (it->stock <= it->reorder_level && !it->on_order) out[k++] = it;
    return k;
}

void inv_free(Inventory *inv) {
    for (int b = 0; b < inv->size; b++) {
        Item *it = inv->buckets[b];
        while (it) { Item *nx = it->next; free(it); it = nx; }
    }
    free(inv->buckets);
    free(inv);
}

int supplier_pick(const Supplier *s, int ns, const int *dist_to_wh, int qty, long *landed) {
    int best = -1; long best_cost = 0;
    for (int i = 0; i < ns; i++) {
        long c = (long)s[i].unit_cost * qty + (long)s[i].per_km * dist_to_wh[s[i].node];
        if (best == -1 || c < best_cost) { best = i; best_cost = c; }
    }
    if (landed) *landed = best_cost;
    return best;
}
