#include <time.h>
#include "oa_hash.h"
#include "hashmap.h"

#define MAX_LINE_LEN 1024

OA_MAP_INIT_UINT64(map64, uint64_t, PRIu64)
OA_MAP_INIT_STR(mapstr, const char *, "s")
OA_MAP_INIT_UINT64_WANG_HASH(map64wang, uint64_t, PRIu64)

static void
test_open_address_hash() {
    oa_hash_t(mapstr) *map_str2 = oa_hash_new(mapstr);
    FILE *f = fopen("oliver_twist_word.txt", "r");
    unsigned char buffer[MAX_LINE_LEN];
    clock_t t1 = clock();
    while(fgets(buffer, MAX_LINE_LEN, f) != NULL) {
        size_t len = strlen(buffer);
        if(buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            char *value = malloc(len*sizeof(unsigned char));
            strcpy(value, buffer);
            oa_hash_map_add(mapstr, map_str2, buffer, (const char *)value);
        }
        else
            printf("too long line\n");
    }
    clock_t t2 = clock();
    double dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("open address hash with string key,insert CPU time used:%0.2fms\n", dur);
    fclose(f);

    f = fopen("oliver_twist_word.txt", "r");
    while(fgets(buffer, MAX_LINE_LEN, f) != NULL) {
        size_t len = strlen(buffer);
        if(buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            assert(oa_hash_get(mapstr, map_str2, buffer) != oa_hash_end(map_str2));
            assert(strcmp(oa_hash_value(map_str2, oa_hash_get(mapstr, map_str2, buffer)), buffer) == 0);
        }
        else
            printf("too long line\n");
    }
    const char *str_key;
    const char *str_value;
    oa_hash_foreach(map_str2, str_key, str_value, {
        free((void *)str_value);
    });
    oa_hash_free(mapstr, map_str2);

    t1 = clock();
    oa_hash_t(map64) *map = oa_hash_new(map64);
    for(uint64_t i = 0;i < 1000000;i++) {
        oa_hash_map_add(map64, map, i, i+1);
    }
    t2 = clock();
    dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("open address hash with uint64_t key,insert CPU time used:%0.2fms\n", dur);
    for(uint64_t i = 0;i < 1000000;i++) {
        assert(oa_hash_get(map64, map, i) != oa_hash_end(map));
        assert(oa_hash_value(map, oa_hash_get(map64, map, i)) == i+1);
    }
    oa_hash_free(map64, map);

    t1 = clock();
    oa_hash_t(map64) *map2 = oa_hash_new(map64);
    for(uint64_t i = 0;i < 1000000U;i++) {
        uint64_t key = i << 32U | 1U;
        uint64_t value = i+1U;
        oa_hash_map_add(map64, map2, key, value);
    }
    t2 = clock();
    dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("open address hash with uint64_t key of same low bits,insert CPU time used:%0.2fms\n", dur);
    for(uint64_t i = 0;i < 1000000U;i++) {
        uint64_t key = i << 32U | 1U;
        uint64_t value = i+1U;
        uint32_t idx = oa_hash_get(map64, map2, key);
        assert(oa_hash_value(map2, idx) == value);
    }
    oa_hash_free(map64, map2);

    t1 = clock();
    oa_hash_t(map64wang) *map_wang = oa_hash_new(map64wang);
    for(uint64_t i = 0;i < 1000000U;i++) {
        uint64_t key = i << 32U | 1U;
        uint64_t value = i+1U;
        oa_hash_map_add(map64wang, map_wang, key, value);
    }
    t2 = clock();
    dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("open address wang hash with uint64_t key of same low bits,insert CPU time used:%0.2fms\n", dur);
    for(uint64_t i = 0;i < 1000000U;i++) {
        uint64_t key = i << 32U | 1U;
        uint64_t value = i+1U;
        uint32_t idx = oa_hash_get(map64wang, map_wang, key);
        assert(oa_hash_value(map_wang, idx) == value);
    }
    oa_hash_free(map64wang, map_wang);
}

static int
compare_str_cb(const void *key1, const void *key2) {
    return strcmp((char *)key1, (char *)key2) == 0;
}

static void *
copy_str_key_cb(const void *key) {
    char *new_key = malloc(strlen((char *)key) + 1);
    strcpy(new_key, (char *)key);
    return (void *)new_key;
}

static void *
copy_str_val_cb(const void *val) {
    int *new_val = malloc(sizeof(int));
    *new_val = *((int *)val);
    return (void *)new_val;
}

static void
free_str_key_cb(void *key) {
    if(key)
        free(key);
}

static void
free_str_val_cb(void *val) {
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

MapType uint64_key_hash_type = {
    hash_cb,       //hash_function
    compare_cb,    //key_cmp
    copy_key_cb,   //copy_key
    copy_val_cb,   //copy_val
    free_key_cb,   //key_destructor
    free_val_cb,   //val_destructor
};

static void
test_link_hash() {
    HashMap *m = new_hashmap(&str_key_hash_type);
    FILE *f = fopen("oliver_twist_word.txt", "r");
    char buffer[MAX_LINE_LEN];
    clock_t t1 = clock();
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
    clock_t t2 = clock();
    double dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("link hash with string key,insert CPU time used:%0.2fms\n", dur);
    
    t1 = clock();
    HashMap *m2 = new_hashmap(&uint64_key_hash_type);
    for(uint64_t i = 0;i < 1000000;i++) {
        int val = 1;
        add_hashmap(m2, (void *)&i, (void *)&val);
    }
    t2 = clock();
    dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("link hash with uint64_t key,insert CPU time used:%0.2fms\n", dur);

    t1 = clock();
    HashMap *m3 = new_hashmap(&uint64_key_hash_type);
    for(uint64_t i = 0;i < 1000000;i++) {
        uint64_t key = i << 32 | 1;
        int val = i+1;
        add_hashmap(m3, (void *)&key, (void *)&val);
    }
    t2 = clock();
    dur = 1000.0*(t2-t1)/CLOCKS_PER_SEC;
    printf("link hash with uint64_t key of same low bits,insert CPU time used:%0.2fms\n", dur);
}

int main() {
    test_link_hash();   
    test_open_address_hash();   
}
