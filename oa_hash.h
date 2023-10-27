#ifndef __OA_HASH_H__
#define __OA_HASH_H__

#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

typedef uint32_t OaHashInt;
typedef uint32_t OaFlagsInt;
typedef const char *OaStrKey;

#define SLOT_INIT_NUM 4

#define WORD_IDX(i) ((i) >> 4U)
#define BIT_IDX(i) (0xFU & (i))
#define SET_DEL(flags, i) (flags[WORD_IDX(i)] |= (1U << (BIT_IDX(i) << 1U)))
#define CLEAR_BOTH_DEL_EMPTY(flags, i) (flags[WORD_IDX(i)] &= ~(3U << (BIT_IDX(i) << 1U)))
#define SET_EXIST(flags, i) CLEAR_BOTH_DEL_EMPTY(flags, i)
#define IS_DEL_OR_EMPTY(flags, i) ((flags[WORD_IDX(i)] >> (BIT_IDX(i) << 1U)) & 3U)
#define IS_EXIST(flags, i) (!IS_DEL_OR_EMPTY(flags, i))

#define calc_upper_limit(slot_size) (OaHashInt)((slot_size) * 0.77 + 0.5)
#define calc_flags_byte_num(slot_size) (WORD_IDX((slot_size) - 1) + 1) * sizeof(OaFlagsInt)
#define clear_flags(flags, byte_num) (memset((flags), 0xaa, (byte_num)))

#define OA_HASH_TYPE(name, key_t, value_t)                                                \
    typedef struct {                                                                      \
        OaHashInt slot_size;                                                              \
        OaHashInt size;                                                                   \
        OaHashInt occupied_size;                                                          \
        OaHashInt upper_limit;                                                            \
        OaFlagsInt *flags;                                                                \
        key_t *keys;                                                                      \
        value_t *values;                                                                  \
    } OaHash##name;

#define OA_HASH_DEFINE_METHOD(name, SCOPE, key_t, value_t,                                \
        hash_func, hash_equal, copy_key, need_free_key, key_format, value_format, is_map) \
    SCOPE OaFlagsInt *                                                                    \
    oa_##name##_init_flags(OaHashInt slot_size) {                                         \
        size_t num = calc_flags_byte_num(slot_size);                                      \
        OaFlagsInt *flags = malloc(num);                                                  \
        clear_flags(flags, num);                                                    \
        return flags;                                                                     \
    }                                                                                     \
    SCOPE OaHash##name *                                                                  \
    oa_##name##_new() {                                                                   \
        OaHash##name *h = malloc(sizeof(OaHash##name));                                   \
        h->slot_size = SLOT_INIT_NUM;                                                     \
        h->size = 0;                                                                      \
        h->occupied_size = 0;                                                             \
        h->upper_limit = calc_upper_limit(h->slot_size);                                  \
        h->keys = calloc(h->slot_size, sizeof(key_t));                                    \
        if(is_map)                                                                        \
            h->values = calloc(h->slot_size, sizeof(value_t));                            \
        else                                                                              \
            h->values = NULL;                                                             \
        h->flags = oa_##name##_init_flags(h->slot_size);                                  \
        return h;                                                                         \
    }                                                                                     \
    SCOPE void                                                                            \
    oa_##name##_print(OaHash##name *h) {                                                  \
        printf("slot_size:%"PRIu32"\n", h->slot_size);                                    \
        printf("size:%"PRIu32"\n", h->size);                                              \
        printf("occupied_size:%"PRIu32"\n", h->occupied_size);                                              \
        printf("flags:\n");                                                               \
        for(int i = WORD_IDX(h->slot_size - 1);i >= 0;i--) {                              \
            printf(" %#x\n", h->flags[i]);                                                \
        }                                                                                 \
        for(OaHashInt i = 0;i < h->slot_size;i++) {                                       \
            if(is_map)                                                                    \
                printf("idx:%"PRIu32",key:%"key_format",value:%"value_format",flag:%u\n", \
                    i, h->keys[i], h->values[i], IS_EXIST(h->flags, i));                  \
            else                                                                          \
                printf("idx:%"PRIu32",key:%"key_format",flag:%u\n", \
                    i, h->keys[i], IS_EXIST(h->flags, i));                  \
        }                                                                                 \
    }                                                                                     \
    SCOPE void                                                                            \
    oa_##name##_free(OaHash##name *h) {                                                   \
        if(h) {                                                                           \
            if(need_free_key) {                                                           \
                for(OaHashInt i = 0;i < h->slot_size;i++) {                               \
                    if(IS_DEL_OR_EMPTY(h->flags, i)) {                                    \
                        continue;                                                         \
                    }                                                                     \
                    free((void *)h->keys[i]);                                                     \
                }                                                                         \
            }                                                                             \
            free(h->keys);                                                                \
            free(h->values);                                                              \
            free(h);                                                                      \
        }                                                                                 \
    }                                                                                     \
    SCOPE void                                                                            \
    oa_##name##_rehash(OaHash##name *h, OaHashInt new_num) {                              \
        OaFlagsInt *new_flags = oa_##name##_init_flags(new_num);                          \
        assert(new_flags);                                                                \
        if(new_num > h->slot_size) {                                                      \
            h->keys = realloc(h->keys, new_num * sizeof(key_t));                          \
            assert(h->keys);                                                              \
            if(is_map) {                                                                  \
                h->values = realloc(h->values, new_num * sizeof(value_t));                \
                assert(h->values);                                                        \
            }                                                                             \
        }                                                                                 \
        for(OaHashInt i = 0;i < h->slot_size;i++) {                                       \
            if(IS_DEL_OR_EMPTY(h->flags, i)) {                                            \
                continue;                                                                 \
            }                                                                             \
            key_t old_key = h->keys[i];                                                   \
            value_t old_value;                                                            \
            if(is_map)                                                                    \
                old_value = h->values[i];                                                 \
            SET_DEL(h->flags, i);                                                         \
            while(true) {                                                                 \
                OaHashInt new_slot_idx = hash_func(old_key, new_num);                     \
                OaHashInt step = 0;                                                       \
                while(IS_EXIST(new_flags, new_slot_idx)) {                                \
                    new_slot_idx = (new_slot_idx + (++step)) & (new_num - 1);             \
                }                                                                         \
                if(new_slot_idx < h->slot_size && IS_EXIST(h->flags, new_slot_idx)) {     \
                    key_t tmp_key = h->keys[new_slot_idx];                                \
                    value_t tmp_value;                                                    \
                    if(is_map)                                                            \
                        tmp_value = h->values[new_slot_idx];                              \
                    h->keys[new_slot_idx] = old_key;                                      \
                    if(is_map)                                                            \
                        h->values[new_slot_idx] = old_value;                              \
                    SET_DEL(h->flags, new_slot_idx);                                      \
                    SET_EXIST(new_flags, new_slot_idx);                                   \
                    old_key = tmp_key;                                                    \
                    if(is_map)                                                            \
                        old_value = tmp_value;                                            \
                    continue;                                                             \
                }                                                                         \
                h->keys[new_slot_idx] = old_key;                                          \
                if(is_map)                                                                \
                    h->values[new_slot_idx] = old_value;                                  \
                SET_EXIST(new_flags, new_slot_idx);                                       \
                break;                                                                    \
            }                                                                             \
        }                                                                                 \
        if(new_num < h->slot_size) {                                                      \
            h->keys = realloc(h->keys, new_num * sizeof(key_t));                          \
            if(is_map)                                                                    \
                h->values = realloc(h->values,new_num * sizeof(value_t));                 \
        }                                                                                 \
        free(h->flags);                                                                   \
        h->flags = new_flags;                                                             \
        h->slot_size = new_num;                                                           \
        h->occupied_size = h->size;                                                       \
        h->upper_limit = calc_upper_limit(h->slot_size);                                  \
    }                                                                                     \
    SCOPE OaHashInt                                                                       \
    oa_##name##_add_key(OaHash##name *h, key_t key) {                                     \
        if(h->occupied_size >= h->upper_limit) {                                          \
            if(h->size >= (h->slot_size >> 1U)) {                                         \
                if(h->slot_size > (UINT32_MAX >> 1U)) {                                   \
                    return h->slot_size;                                                  \
                }                                                                         \
                oa_##name##_rehash(h, h->slot_size << 1U);                                \
            }                                                                             \
            else {                                                                        \
                oa_##name##_rehash(h, h->slot_size);                                      \
            }                                                                             \
        }                                                                                 \
                                                                                          \
        OaHashInt slot_idx = hash_func(key, h->slot_size);                                \
        OaHashInt step = 0;                                                               \
        while(IS_EXIST(h->flags, slot_idx) && !hash_equal(key, h->keys[slot_idx])) {      \
            slot_idx = (slot_idx + (++step)) & (h->slot_size - 1);                        \
        }                                                                                 \
        bool is_key_exist = IS_EXIST(h->flags, slot_idx);                                 \
        h->keys[slot_idx] = copy_key(key);                                                \
        SET_EXIST(h->flags, slot_idx);                                                    \
        if(!is_key_exist) {                                                               \
            h->size++;                                                                    \
            h->occupied_size++;                                                           \
        }                                                                                 \
        return slot_idx;                                                                  \
    }                                                                                     \
    SCOPE bool                                                                            \
    oa_##name##_map_add(OaHash##name *h, key_t key, value_t value) {                      \
        assert(is_map);                                                                   \
        OaHashInt slot_idx = oa_##name##_add_key(h, key);                                 \
        if(slot_idx == h->slot_size)                                                      \
            return false;                                                                 \
        h->values[slot_idx] = value;                                                      \
        return true;                                                                      \
    }                                                                                     \
    SCOPE bool                                                                            \
    oa_##name##_set_add(OaHash##name *h, key_t key) {                                     \
        assert(!is_map);                                                                  \
        OaHashInt slot_idx = oa_##name##_add_key(h, key);                                 \
        if(slot_idx == h->slot_size)                                                      \
            return false;                                                                 \
        return true;                                                                      \
    }                                                                                     \
    SCOPE OaHashInt                                                                       \
    oa_##name##_get(OaHash##name *h, key_t key) {                                         \
        OaHashInt slot_idx = hash_func(key, h->slot_size);                                \
        OaHashInt step = 0;                                                               \
        while(IS_EXIST(h->flags, slot_idx) && !hash_equal(key, h->keys[slot_idx])) {      \
            slot_idx = (slot_idx + (++step)) & (h->slot_size - 1);                        \
        }                                                                                 \
        if(!IS_EXIST(h->flags, slot_idx)) {                                               \
            return h->slot_size;                                                          \
        }                                                                                 \
        return slot_idx;                                                                  \
    }                                                                                     \
    SCOPE void                                                                            \
    oa_##name##_delete(OaHash##name *h, key_t key) {                                      \
        OaHashInt slot_idx = hash_func(key, h->slot_size);                                \
        if(!IS_EXIST(h->flags, slot_idx))                                                 \
            return;                                                                       \
        SET_DEL(h->flags, slot_idx);                                                      \
        --h->size;                                                                        \
        OaHashInt shrink_limit = h->slot_size >> 3U;                                      \
        if(shrink_limit > 0 && h->size <= shrink_limit)                                   \
            oa_##name##_rehash(h, h->slot_size >> 1U);                                    \
    }                                                                                     \
    SCOPE void                                                                            \
    oa_##name##_clear(OaHash##name *h) {                                                  \
        if(h && h->flags) {                                                               \
            size_t num = calc_flags_byte_num(h->slot_size);                                   \
            clear_flags(h->flags, num);                                                   \
            h->size = 0;                                                                  \
            h->occupied_size = 0;                                                         \
        }                                                                                 \
    }                                                                                     \

#define oa_uint32_hash_func(key, slot_size) (((OaHashInt)(key)) & ((slot_size) - 1))
#define oa_uint32_hash_equal(key1, key2) ((key1) == (key2))
#define oa_uint64_hash_func(key, slot_size) (((OaHashInt)((key)>>33^(key)^(key)<<11)) & ((slot_size) - 1))
#define oa_uint64_hash_equal(key1, key2) ((key1) == (key2))
static inline
OaHashInt oa_hash_string(const char *s)
{
    OaHashInt h = (OaHashInt)*s;
    if(h) {
        for(++s;*s;++s)
            h = (h << 5) - h + (OaHashInt)*s;
    }
    return h;
}
#define oa_str_hash_func(key, slot_size) (oa_hash_string(key) & ((slot_size) - 1))
#define oa_str_hash_equal(key1, key2) (strcmp(key1, key2) == 0)

static inline OaHashInt
oa_Wang_hash_uint32(OaHashInt key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}
#define oa_uint32_Wang_hash_func(key, slot_size) (oa_Wang_hash_uint32((OaHashInt)key) & ((slot_size) - 1))

static inline OaHashInt
oa_Wang_hash_uint64(uint64_t key) {
	key = ~key + (key << 21);
	key = key ^ key >> 24;
	key = (key + (key << 3)) + (key << 8);
	key = key ^ key >> 14;
	key = (key + (key << 2)) + (key << 4);
	key = key ^ key >> 28;
	key = key + (key << 31);
	return (OaHashInt)key;
}
#define oa_uint64_Wang_hash_func(key, slot_size) (oa_Wang_hash_uint64(key) & ((slot_size) - 1))

#define oa_hash_t(name) OaHash##name
#define oa_hash_new(name) oa_##name##_new()
#define oa_hash_free(name, h) oa_##name##_free(h)
#define oa_hash_map_add(name, h, key, value) oa_##name##_map_add(h, key, value)
#define oa_hash_set_add(name, h, key) oa_##name##_set_add(h, key)
#define oa_hash_delete(name, h, key) oa_##name##_delete(h, key)
#define oa_hash_clear(name, h) oa_##name##_clear(h)
#define oa_hash_print(name, h) oa_##name##_print(h)
#define oa_hash_get(name, h, key) oa_##name##_get(h, key)
#define oa_hash_begin(h) (OaHashInt)0U
#define oa_hash_end(h) ((h)->slot_size)
#define oa_hash_key(h, i) ((h)->keys[i])
#define oa_hash_value(h, i) ((h)->values[i])
#define oa_hash_exist(h, i) IS_EXIST((h)->flags, (i))
#define oa_hash_size(h) ((h)->size)
#define oa_hash_slot_size(h) ((h)->slot_size)
#define oa_hash_foreach(h, key_var, value_var, code) do {                                 \
    for(OaHashInt i = oa_hash_begin(h);i < oa_hash_end(h);i++) {                          \
        if(!oa_hash_exist(h, i)) continue;                                                \
        (key_var) = oa_hash_key(h, i);                                                    \
        (value_var) = oa_hash_value(h, i);                                                \
        code;                                                                             \
    }                                                                                     \
} while(0)
#define oa_hash_foreach_value(h, value_var, code) do {                                    \
    for(OaHashInt i = oa_hash_begin(h);i < oa_hash_end(h);i++) {                          \
        if(!oa_hash_exist(h, i)) continue;                                                \
        (value_var) = oa_hash_value(h, i);                                                \
        code;                                                                             \
    }                                                                                     \
} while(0)

static inline const char *
oa_copy_str_key(const char *key) {
    char *new_key = malloc(strlen(key) + 1);
    strcpy(new_key, key);
    return (const char *)new_key;
}

#define oa_copy_uint_key(key) (key)

#define OA_MAP_INIT_UINT64(name, value_t, value_format)                                   \
    OA_HASH_TYPE(name, uint64_t, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint64_t, value_t,                         \
        oa_uint64_hash_func, oa_uint64_hash_equal, oa_copy_uint_key, false, PRIu64, value_format, true)            \

#define OA_SET_INIT_UINT64(name)                                                          \
    OA_HASH_TYPE(name, uint64_t, uint8_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint64_t, uint8_t,                         \
        oa_uint64_hash_func, oa_uint64_hash_equal, oa_copy_uint_key, false, PRIu64, "c", false)                    \

#define OA_MAP_INIT_UINT64_WANG_HASH(name, value_t, value_format)                                   \
    OA_HASH_TYPE(name, uint64_t, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint64_t, value_t,                         \
        oa_uint64_Wang_hash_func, oa_uint64_hash_equal, oa_copy_uint_key, false, PRIu64, value_format, true)            \

#define OA_SET_INIT_UINT64_WANG_HASH(name)                                                          \
    OA_HASH_TYPE(name, uint64_t, uint8_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint64_t, uint8_t,                         \
        oa_uint64_Wang_hash_func, oa_uint64_hash_equal, oa_copy_uint_key, false, PRIu64, "c", false)                    \

#define OA_MAP_INIT_UINT32(name, value_t, value_format)                                   \
    OA_HASH_TYPE(name, uint32_t, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint32_t, value_t,                         \
        oa_uint32_hash_func, oa_uint32_hash_equal, oa_copy_uint_key, false, PRIu32, value_format, true)            \

#define OA_SET_INIT_UINT32(name)                                                          \
    OA_HASH_TYPE(name, uint32_t, uint8_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint32_t, uint8_t,                         \
        oa_uint32_hash_func, oa_uint32_hash_equal, oa_copy_uint_key, false, PRIu32, "c", false)                    \

#define OA_MAP_INIT_UINT32_WANG_HASH(name, value_t, value_format)                                   \
    OA_HASH_TYPE(name, uint32_t, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint32_t, value_t,                         \
        oa_uint32_Wang_hash_func, oa_uint32_hash_equal, oa_copy_uint_key, false, PRIu32, value_format, true)            \

#define OA_SET_INIT_UINT32_WANG_HASH(name)                                                          \
    OA_HASH_TYPE(name, uint32_t, uint8_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, uint32_t, uint8_t,                         \
        oa_uint32_Wang_hash_func, oa_uint32_hash_equal, oa_copy_uint_key, false, PRIu32, "c", false)                    \

#define OA_MAP_INIT_STR(name, value_t, value_format)                                      \
    OA_HASH_TYPE(name, OaStrKey, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, OaStrKey, value_t,                         \
        oa_str_hash_func, oa_str_hash_equal, oa_copy_str_key, true, "s", value_format, true)                     \

#define OA_SET_INIT_STR(name, value_t, value_format)                                      \
    OA_HASH_TYPE(name, OaStrKey, value_t)                                                 \
    OA_HASH_DEFINE_METHOD(name, static inline, OaStrKey, value_t,                         \
        oa_str_hash_func, oa_str_hash_equal, oa_copy_str_key, true, "s", value_format, false)                     \

#endif
