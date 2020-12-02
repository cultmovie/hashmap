#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "hashmap.h"

static int _add_slot(HashMap *m, Slot **slots, int slot_size, void *key, void *value);
static void _move_slot(HashMap *m, Slot **slots, int slot_size, void *key, void *value);
static void rehash(HashMap *m, int new_size);

HashMap *new_hashmap(MapType *type){
    HashMap *m = (HashMap *)malloc(sizeof(HashMap));
    m->slots = (Slot **)calloc(INIT_SIZE, sizeof(Slot *));
    m->count = 0;
    m->slots_size = INIT_SIZE;
    m->type = type;
    return m;
}

void free_hashmap(HashMap *m){
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        if(p == NULL)
            continue;
        Slot *tmp = NULL;
        while(p) {
            tmp = p;
            p = p->next;
            free_key(m, tmp);
            free_val(m, tmp);
            free(tmp);
        }
        m->slots[i] = NULL;
    }
    free(m->slots);
    free(m);
}

int add_hashmap(HashMap *m, void *key, void *value){
    if(m->count >= m->slots_size && m->slots_size <= INT_MAX/2){
        rehash(m, m->slots_size * 2);
    }
    return _add_slot(m, m->slots, m->slots_size, key, value);
}

void *query_hashmap(HashMap *m, const void *key){
    uint64_t hash_key = gen_hash_key(m, key);
    int h = HASH(hash_key, m->slots_size);
    Slot *p = m->slots[h];
    while(p){
        if(key == p->key || cmp_key(m, p->key, key))
            return p->value;
        p = p->next;
    }
    return NULL;
}

int remove_hashmap(HashMap *m, const void *key){
    uint64_t hash_key = gen_hash_key(m, key);
    int h = HASH(hash_key, m->slots_size);
    Slot *p = m->slots[h];
    Slot *prior = NULL;
    bool is_find = false;
    while(p){
        if(key == p->key || cmp_key(m, p->key, key)){
            is_find = true;
            break;
        }
        prior = p;
        p = p->next;
    }
    if(!is_find)
        return FAILED;
    if(prior){
        prior->next = p->next;
        free_key(m, p);
        free_val(m, p);
        free(p);
    }
    else {
        m->slots[h] = p->next;
        free_key(m, p);
        free_val(m, p);
        free(p);
    }
    m->count--;
    if(m->count < m->slots_size / 4)
        rehash(m, m->slots_size / 2);
    return SUCC;
}

void traverse_hashmap(HashMap *m, traverse_hook hook, void *extra){
	for(int i = 0;i < m->slots_size;i++){
		Slot *p = m->slots[i];
		if(p == NULL)
			continue;
		while(p){
			hook(p->key, p->value, extra);
			p = p->next;
		}
	}
}

void get_hashmap_stats(HashMap *m, Stats *stats) {
    stats->count = m->count;
    stats->slots_size = m->slots_size;
    stats->load_factor = ((double)m->count) / m->slots_size;
}

uint64_t bkdrhash_hashmap(const void *key) {
    uint64_t seed = 31;
    uint64_t hash = 0;
    char *skey = (char *)key;
    while(*skey)
        hash = hash * seed + (*skey++);
    return hash & 0x7FFFFFFFFFFFFFFF;
}

/* private function */
static int _add_slot(HashMap *m, Slot **slots, int slot_size, void *key, void *value) {
    uint64_t hash_key = gen_hash_key(m, key);
    int h = HASH(hash_key, slot_size);
    Slot *head = slots[h];
    if(head == NULL) {
        head = (Slot *)malloc(sizeof(Slot));
        copy_key(m, head, key);
        copy_val(m, head, value);
        head->next = NULL;
        slots[h] = head;
        m->count++;
        return ADD;
    }
    Slot *p = slots[h];
    while(p){
        if(key == p->key || cmp_key(m, p->key, key)) {
            free_val(m, p);
            copy_val(m, p, value);
            return REPLACE;
        }
        p = p->next;
    }
    Slot *new_slot = (Slot *)malloc(sizeof(Slot));
    copy_key(m, new_slot, key);
    copy_val(m, new_slot, value);
    slots[h] = new_slot;
    new_slot->next = head;
    m->count++;
    return ADD;
}

static void _move_slot(HashMap *m, Slot **slots, int slot_size, void *key, void *value) {
    uint64_t hash_key = gen_hash_key(m, key);
    int h = HASH(hash_key, slot_size);
    Slot *head = slots[h];
    if(head == NULL) {
        head = (Slot *)malloc(sizeof(Slot));
        head->key = key;
        head->value = value;
        head->next = NULL;
        slots[h] = head;
        return;
    }
    Slot *new_slot = (Slot *)malloc(sizeof(Slot));
    new_slot->key = key;
    new_slot->value = value;
    new_slot->next = NULL;
    Slot *p = head;
    while(p) {
        if(p->next)
            p = p->next;
        else {
            p->next = new_slot;
            break;
        }
    }
}

static void rehash(HashMap *m, int new_size){
    assert(new_size != m->slots_size);
    Slot **new_slots = (Slot **)calloc(new_size, sizeof(Slot *));
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        Slot *tmp = NULL;
        while(p){
            _move_slot(m, new_slots, new_size, p->key, p->value);
            tmp = p;
            p = p->next;
            free(tmp);
        }
    }
    free(m->slots);
    m->slots = new_slots;
    m->slots_size = new_size;
}

#ifdef TEST_MAIN

static void dump_hashmap(HashMap *m, int key_type){
    printf("slots_size:%d\n", m->slots_size);
    printf("count:%d\n", m->count);
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        if(p == NULL)
            continue;
        printf("slot idx:%d\n", i);
        while(p){
            if(key_type == 1)
                printf("------key:%ld,value:%d\n", *((uint64_t *)p->key), *((int *)p->value));
            else
                printf("------key:%s,value:%d\n", (char *)p->key, *((int *)p->value));
            p = p->next;
        }
    }
    printf("\n");
}

static void print_chains_len(HashMap *m) {
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        if(p == NULL) {
            printf("%d\n", 0);
            continue;
        }
        int chain_len = 0;
        while(p){
            chain_len++;
            p = p->next;
        }
        printf("%d\n", chain_len);
    }
    printf("\n");
}

uint64_t hash_cb(const void *key) {
    return *((uint64_t *)key);
}

int compare_cb(const void *key1, const void *key2) {
    return *((uint64_t *)key1) == *((uint64_t *)key2);
}

void *copy_key_cb(const void *key) {
    uint64_t *new_key = malloc(sizeof(uint64_t));
    *new_key = *((uint64_t *)key);
    return (void *)new_key;
}

void *copy_val_cb(const void *val) {
    int *new_val = malloc(sizeof(int));
    *new_val = *((int *)val);
    return (void *)new_val;
}

void free_key_cb(void *key) {
    if(key)
        free(key);
}

void free_val_cb(void *val) {
    if(val)
        free(val);
}

MapType int_key_hash_type = {
    hash_cb,       //hash_function
    compare_cb,    //key_cmp
    copy_key_cb,   //copy_key
    copy_val_cb,   //copy_val
    free_key_cb,   //key_destructor
    free_val_cb,   //val_destructor
};

int compare_str_cb(const void *key1, const void *key2) {
    return strcmp((char *)key1, (char *)key2) == 0;
}

void *copy_str_key_cb(const void *key) {
    char *new_key = malloc(strlen((char *)key) + 1);
    strcpy(new_key, (char *)key);
    return (void *)new_key;
}

void *copy_str_val_cb(const void *val) {
    int *new_val = malloc(sizeof(int));
    *new_val = *((int *)val);
    return (void *)new_val;
}

void free_str_key_cb(void *key) {
    if(key)
        free(key);
}

void free_str_val_cb(void *val) {
    if(val)
        free(val);
}

MapType str_key_hash_type = {
    bkdrhash_hashmap,  //hash_function
    compare_str_cb,    //key_cmp
    copy_str_key_cb,   //copy_key
    copy_str_val_cb,   //copy_val
    free_str_key_cb,   //key_destructor
    free_str_val_cb,   //val_destructor
};

void test_int_key() {
    HashMap *m = new_hashmap(&int_key_hash_type);   
    int num1 = rand() % 100000;
    for(uint64_t i=0;i<num1;i++) {
        int p2 = 1;
        add_hashmap(m, (void *)&i, (void *)&p2);
    }
    dump_hashmap(m, 1);
    Stats stats;
    get_hashmap_stats(m, &stats);
    printf("count:%d,slots_size:%d,load_factor:%lf\n", stats.count, stats.slots_size, stats.load_factor);
    int num2 = rand() % 100;
    for(uint64_t i=0;i<num2;i++)
        remove_hashmap(m, (void *)&i);
    dump_hashmap(m, 1);
    printf("count:%d,slots_size:%d,load_factor:%lf\n", stats.count, stats.slots_size, stats.load_factor);
    free_hashmap(m);
}

#define MAX_LINE_LEN 1024
/*
 *chain len 1,key count:11060 52.4%
 *chain len 2,key count:3607  34.2%
 *chain len 3,key count:737   10.4%
 *chain len 4,key count:133   2.5%
 *chain len 5,key count:13    0.3%
 *chain len 6,key count:3     0.08%
 *chain len 7,key count:1     0.005%
 * */
void test_str_key() {
    HashMap *m = new_hashmap(&str_key_hash_type);
    FILE *f = fopen("oliver_twist_word.txt", "r");
    char buffer[MAX_LINE_LEN];
    while(fgets(buffer, MAX_LINE_LEN, f) != NULL) {
        size_t len = strlen(buffer);
        if(buffer[len - 1] == '\n') {
            int val = 1;
            buffer[len - 1] = '\0';
            add_hashmap(m, (void *)buffer, (void *)&val);
        }
        else
            printf("too long line\n");
    }
    char new_str[] = "hanging";
    int new_val = 10;
    add_hashmap(m, (void *)new_str, (void *)&new_val);
    remove_hashmap(m, (void *)"Toby:");
    dump_hashmap(m, 0);
    fclose(f);
    print_chains_len(m);
    Stats stats;
    get_hashmap_stats(m, &stats);
    printf("count:%d,slots_size:%d,load_factor:%lf\n", stats.count, stats.slots_size, stats.load_factor);
}

void main(){
    srand((unsigned int)time(NULL));
    test_str_key();
}
#endif
