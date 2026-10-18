#ifndef SUPPLY_H
#define SUPPLY_H

/* Inventory: hash table (separate chaining) keyed by SKU -> O(1) average lookup. */
typedef struct Item {
    char sku[12], name[24];
    int stock, reorder_level, reorder_qty, on_order;
    struct Item *next;
} Item;
typedef struct { Item **buckets; int size, count; } Inventory;

Inventory *inv_create(int size);
Item *inv_add(Inventory *inv, const char *sku, const char *name, int stock, int reorder_level, int reorder_qty);
Item *inv_find(const Inventory *inv, const char *sku);
int   inv_take(Inventory *inv, const char *sku, int qty);          /* returns units actually fulfilled */
int   inv_low_stock(const Inventory *inv, Item **out, int max);    /* items at/below reorder level, not yet on order */
void  inv_free(Inventory *inv);

/* Supplier tracking: pick the supplier with the lowest landed cost = goods + transport. */
typedef struct { const char *name; int node, unit_cost, per_km; } Supplier;
int supplier_pick(const Supplier *s, int ns, const int *dist_to_wh, int qty, long *landed);

#endif
