#include "oa_hash.h"

OA_MAP_INIT_UINT64(map64, uint64_t, PRIu64)
OA_MAP_INIT_STR(mapstr, uint64_t, PRIu64)
OA_SET_INIT_UINT64(set64)

int main() {
    oa_hash_t(map64) *m = oa_hash_new(map64);
    oa_hash_map_add(map64, m, 100, 200);
    oa_hash_map_add(map64, m, 101, 201);
    oa_hash_map_add(map64, m, 102, 202);
    oa_hash_map_add(map64, m, 103, 203);
    oa_hash_print(map64, m);
    oa_hash_clear(map64, m);
    oa_hash_delete(map64, m, 101);
    oa_hash_delete(map64, m, 102);
    oa_hash_delete(map64, m, 100);
    oa_hash_delete(map64, m, 103);
    oa_hash_print(map64, m);
    /*for(uint64_t i = 0;i < 17U;i++)
        oa_hash_map_add(map64, m, i+100, i+200);
    for(uint32_t i = 0;i < 9U;i++)
        oa_hash_delete(map64, m, i+100);
    oa_hash_print(map64, m);
    uint64_t idx = oa_hash_get(map64, m, 116);
    if(idx != oa_hash_end(m))
        printf("----idx:%lu,key:%lu,value:%lu\n", idx, oa_hash_key(m, idx), oa_hash_value(m, idx));
    uint64_t k,v;
    oa_hash_foreach(m, k, v, {
        printf("k:%lu,v:%lu\n", k, v);
    });
    oa_hash_foreach_value(m, v, {
        printf("v:%lu\n", v);
    });
    oa_hash_free(map64, m);

    oa_hash_t(mapstr) *map_str = oa_hash_new(mapstr);
    oa_hash_map_add(mapstr, map_str, "hello,world", 1);
    oa_hash_map_add(mapstr, map_str, "raining", 2);
    oa_hash_map_add(mapstr, map_str, "palace,", 2);
    oa_hash_map_add(mapstr, map_str, "'I'll", 2);
    oa_hash_map_add(mapstr, map_str, "wide-skirted;", 2);
    oa_hash_map_add(mapstr, map_str, "considered,", 2);
    oa_hash_map_add(mapstr, map_str, "shouted,", 2);
    oa_hash_print(mapstr, map_str);
    const char *key_str;
    uint64_t value;
    oa_hash_foreach(map_str, key_str, value, {
        printf("k:%s,v:%lu\n", key_str, value);
    });
    oa_hash_free(mapstr, map_str);
    
    oa_hash_t(set64) *set = oa_hash_new(set64);
    oa_hash_set_add(set64, set, 2000);
    oa_hash_set_add(set64, set, 2001);
    oa_hash_set_add(set64, set, 2002);
    oa_hash_print(set64, set);
    oa_hash_free(set64, set);*/

    return 0;
}
