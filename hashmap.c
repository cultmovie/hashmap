#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "hashmap.h"

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

static int _add_hash(HashMap *m, Slot **slots, int slot_size, int *count, void *key, void *value){
    uint64_t hash_key = gen_hash_key(m, key);
    int h = HASH(hash_key, slot_size);
    Slot *head = slots[h];
    if(head == NULL) {
        head = (Slot *)malloc(sizeof(Slot));
        head->key = key;
        head->value = value;
        head->next = NULL;
        slots[h] = head;
        if(count) (*count)++;
        return ADD;
    }
    Slot *p = slots[h];
    while(p){
        if(key == p->key || cmp_key(m, p->key, key)) {
            free_val(m, p);
            p->value = value;
            return REPLACE;
        }
        p = p->next;
    }
    Slot *new_slot = (Slot *)malloc(sizeof(Slot));
    new_slot->key = key;
    new_slot->value = value;
    slots[h] = new_slot;
    new_slot->next = head;
    if(count) (*count)++;
    return ADD;
}

static void rehash(HashMap *m, int new_size){
    assert(new_size != m->slots_size);
    Slot **new_slots = (Slot **)calloc(new_size, sizeof(Slot *));
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        Slot *tmp = NULL;
        while(p){
            _add_hash(m, new_slots, new_size, NULL, p->key, p->value);
            tmp = p;
            p = p->next;
            free(tmp);
        }
    }
    free(m->slots);
    m->slots = new_slots;
    m->slots_size = new_size;
}

int add_hash(HashMap *m, void *key, void *value){
    if(m->count >= m->slots_size && m->slots_size <= INT_MAX/2){
        rehash(m, m->slots_size * 2);
    }
    return _add_hash(m, m->slots, m->slots_size, &m->count, key, value);
}

void *query_hash(HashMap *m, void *key){
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

int remove_hash(HashMap *m, void *key){
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

void dump_hashmap(HashMap *m){
    printf("slots_size:%d\n", m->slots_size);
    printf("count:%d\n", m->count);
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        if(p == NULL)
            continue;
        printf("slot idx:%d\n", i);
        while(p){
            printf("------key:%ld\n", *((uint64_t *)p->key));
            p = p->next;
        }
    }
    printf("\n");
}

uint64_t hash_cb(const void *key) {
    return *((uint64_t *)key);
}

int compare_cb(const void *key1, const void *key2) {
    return *((uint64_t *)key1) == *((uint64_t *)key2);
}

void free_key_cb(void *key) {
    free(key);
}

void free_val_cb(void *val) {
    free(val);
}

MapType int_key_hash_type = {
    hash_cb,   //hash_function
    compare_cb,    //key_cmp
    free_key_cb,   //key_destructor
    free_val_cb,   //val_destructor
};

static char flag = 1;

void main(){
    HashMap *m = new_hashmap(&int_key_hash_type);   
    srand((unsigned int)time(NULL));
    int num1 = rand() % 1000;
    for(int i=0;i<num1;i++) {
        uint64_t *p1 = malloc(sizeof(uint64_t));
        *p1 = i;
        int *p2 = malloc(sizeof(int));
        *p2 = 1;
        if(add_hash(m, (void *)p1, (void *)p2) == REPLACE)
            free(p1);
    }
    dump_hashmap(m);
    int num2 = rand() % 100;
    for(int i=0;i<num2;i++)
        remove_hash(m, (void *)&i);
    dump_hashmap(m);
    free_hashmap(m);
}
