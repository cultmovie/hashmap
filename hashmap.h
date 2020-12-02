#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>

#define INIT_SIZE 2
#define cast(t, exp)    ((t)(exp))
#define HASH(key, slots_size) (cast(int, (key) & ((slots_size) - 1)))

#define FAILED 0
#define SUCC 1
#define REPLACE 2
#define ADD 3

#define gen_hash_key(m, key) \
    (m)->type->hash_function((key))

#define cmp_key(m, key1, key2) \
    (((m)->type->key_cmp) ? ((m)->type->key_cmp((key1), (key2))) : (key1) == (key2))

#define copy_key(m, slot, _key_) do { \
    if((m)->type->copy_key) \
        (slot)->key = (m)->type->copy_key((_key_)); \
    else \
        (slot)->key = (_key_); \
} while(0)

#define copy_val(m, slot, val) do { \
    if((m)->type->copy_val) \
        (slot)->value = (m)->type->copy_val((val)); \
    else \
        (slot)->value = (val); \
} while(0)

#define free_key(m, slot) do { \
    if((m)->type->key_destructor) \
        (m)->type->key_destructor((slot)->key); \
} while(0)

#define free_val(m, slot) do { \
    if((m)->type->val_destructor) \
        (m)->type->val_destructor((slot)->value); \
} while(0)

typedef struct {
    uint64_t (*hash_function)(const void *key);
    int (*key_cmp)(const void *key1, const void *key2);
    void *(*copy_key)(const void *key);
    void *(*copy_val)(const void *val);
    void (*key_destructor)(void *key);
    void (*val_destructor)(void *val);
} MapType;

typedef struct Slot {
    void *key;
    void *value;
    struct Slot *next;
} Slot;

typedef struct {
    MapType *type;
    Slot **slots;
    int count;
    int slots_size;
} HashMap;

typedef struct {
    int count;
    int slots_size;
    double load_factor;
} Stats;

typedef void(*traverse_hook)(void *key, void *value, void *extra);

HashMap *new_hashmap(MapType *type);
void free_hashmap(HashMap *m);
int add_hashmap(HashMap *m, void *key, void *value);
int remove_hashmap(HashMap *m, void *key);
void *query_hashmap(HashMap *m, void *key);
void traverse_hashmap(HashMap *m, traverse_hook hook, void *extra);
void get_hashmap_stats(HashMap *m, Stats *stats);
uint64_t bkdrhash_hashmap(const void *key);

#endif
