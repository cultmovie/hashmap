#include "hashmap.h"

HashMap *new_hashmap(){
    HashMap *m = (HashMap *)malloc(sizeof(HashMap));
    m->slots = (Slot **)calloc(INIT_SIZE, sizeof(Slot *));
    m->count = 0;
    m->slots_size = INIT_SIZE;
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
            free(tmp);
        }
        m->slots[i] = NULL;
    }
    free(m->slots);
    free(m);
}

static void _add_hash(Slot **slots, int slot_size, int *count, KEY_TYPE key, void *value){
    int h = HASH(key, slot_size);
    Slot *p = slots[h];
    if(p == NULL) {
        p = (Slot *)malloc(sizeof(Slot));
        p->key = key;
        p->value = value;
        p->next = NULL;
        slots[h] = p;
        if(count) (*count)++;
        return;
    }
    Slot *tail = NULL;
    while(p){
        if(p->key == key){
            p->value = value;
            return;
        }
        tail = p;
        p = p->next;
    }
    p = (Slot *)malloc(sizeof(Slot));
    p->key = key;
    p->value = value;
    p->next = NULL;
    tail->next = p;
    if(count) (*count)++;
}

static void rehash(HashMap *m, int new_size){
    assert(new_size != m->slots_size);
    Slot **new_slots = (Slot **)calloc(new_size, sizeof(Slot *));
    for(int i = 0;i < m->slots_size;i++){
        Slot *p = m->slots[i];
        Slot *tmp = NULL;
        while(p){
            _add_hash(new_slots, new_size, NULL, p->key, p->value);
            tmp = p;
            p = p->next;
            free(tmp);
        }
    }
    free(m->slots);
    m->slots = new_slots;
    m->slots_size = new_size;
}

void add_hash(HashMap *m, KEY_TYPE key, void *value){
    if(m->count >= m->slots_size && m->slots_size <= INT_MAX/2){
        rehash(m, m->slots_size * 2);
    }
    _add_hash(m->slots, m->slots_size, &m->count, key, value);
}

void *query_hash(HashMap *m, KEY_TYPE key){
    int h = HASH(key, m->slots_size);
    Slot *p = m->slots[h];
    while(p){
        if(p->key == key)
            return p->value;
        p = p->next;
    }
    return NULL;
}

void *remove_hash(HashMap *m, KEY_TYPE key){
    int h = HASH(key, m->slots_size);
    Slot *p = m->slots[h];
    Slot *prior = NULL;
    bool is_find = false;
    while(p){
        if(p->key == key){
            is_find = true;
            break;   
        }
        prior = p;
        p = p->next;
    }
    if(!is_find)
        return NULL;
    void *value = p->value;
    if(prior){
        prior->next = p->next;
        free(p);
        p = NULL;
    }
    else {
        m->slots[h] = p->next;
        free(p);
        p = NULL;
    }
    m->count--;
    if(m->count < m->slots_size / 4)
        rehash(m, m->slots_size / 2);
    return value;
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
            printf("------key:%ld\n", p->key);
            p = p->next;
        }
    }
    printf("\n");
}

static char flag = 1;

void main(){
    HashMap *m = new_hashmap();   
    srand((unsigned int)time(NULL));
    int num1 = rand() % 1000;
    for(int i=0;i<num1;i++)
        add_hash(m, i, (void *)&flag);
    //dump_hashmap(m);
    int num2 = rand() % 100;
    for(int i=0;i<num2;i++)
        remove_hash(m, i);
    //dump_hashmap(m);
    free_hashmap(m);
}
