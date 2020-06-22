#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#define INIT_SIZE 2
#define cast(t, exp)    ((t)(exp))
#define KEY_TYPE uint64_t
#define HASH(key, slots_size) (cast(int, (key) & ((slots_size) - 1)))

typedef struct Slot {
    KEY_TYPE key;
    void *value;
    struct Slot *next;
} Slot;

typedef struct {
    Slot **slots;
    int count;
    int slots_size;
} HashMap;

typedef void(*traverse_hook)(KEY_TYPE key, void *value, void *extra);

HashMap *new_hashmap();
void free_hashmap(HashMap *m);
void *add_hash(HashMap *m, KEY_TYPE key, void *value);
void *remove_hash(HashMap *m, KEY_TYPE key);
void *query_hash(HashMap *m, KEY_TYPE key);
void traverse_hashmap(HashMap *m, traverse_hook hook, void *extra);

#endif
