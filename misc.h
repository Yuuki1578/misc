/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under the MIT License. All rights reserved.

######          ######   ###     #############         ###############
######          ######   ###   ################     #####################
### ###        ### ###   ###   ###           ###   ###                ###
###  ###      ###  ###   ###   ###                 ###
###   ###    ###   ###   ###   ###                 ###
###    ###  ###    ###   ###     #############     ###
###     ######     ###   ###                 ###   ###
###                ###   ###                 ###   ###
###                ###   ###                 ###   ###
###                ###   ###   ###           ###   ###                ###
###                ###   ###   #################     ####################
###                ###   ###     #############         ###############


*/

#ifndef MISC_H
#define MISC_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RemainsOfArena(a)   ((a)->total - (a)->offset)
#define MISC_VOIDPTR(expr)  ((void *)(expr))
#define MISC_ARENA_PAGESIZE (1ULL << 12ULL)
#define MISC_ARSTACK        (0x1)
#define MISC_ARHEAP         (0x10)
#define MISC_ARNOGROW       (0x100)
#define MISC_ARDEFAULT      (MISC_ARHEAP)

typedef struct Arena Arena;
struct Arena {
    Arena   *next;
    uint32_t total, offset, flags;
};

#define MISC_ARRAY_RESERVE (8)

#define Array(Type) \
    struct { \
        Type *items; \
        uint32_t cap; \
        uint32_t len; \
    }

#define IsArrayEmpty(array)   ((array) != NULL ? ((array)->items == NULL && !(array)->cap) : 0)
#define RemainsOfArray(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define ResizeArrayCheck(array, N, ok) \
    do { \
        if ((N) <= 0) { \
            free((array)->items); \
            (array)->items = NULL; \
            (array)->cap   = 0; \
            (array)->len   = 0; \
            *(ok)          = 1; \
        } else { \
            void *tmp; \
            if ((array)->items == NULL) { \
                tmp = calloc((N), sizeof *(array)->items); \
            } else { \
                tmp = realloc((array)->items, (N) * sizeof *(array)->items); \
            } \
            if (tmp != NULL) { \
                *(ok)          = 1; \
                (array)->items = tmp; \
                (array)->cap   = (N); \
                if ((N) < (array)->len) { \
                    (array)->len = (N); \
                } \
            } else { \
                *(ok) = 0; \
            } \
        } \
    } while (0)

#define AppendArrayCheck(array, item, ok) \
    do { \
        if ((array)->cap <= (array)->len) { \
            ResizeArrayCheck(array, (array)->cap + MISC_ARRAY_RESERVE, ok); \
        } \
        if (*(ok)) { \
            (array)->items[(array)->len++] = (item); \
        } \
    } while (0)

#define ExtendArrayCheck(array, manyPtr, N, ok) \
    do { \
        if ((manyPtr) != NULL && (N) > 0) { \
            if (IsArrayEmpty(array) || RemainsOfArray(array) <= (N)) { \
                ResizeArrayCheck(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok); \
                if (!*(ok)) { \
                    break; \
                } \
            } \
            memmove((array)->items + (array)->len, (manyPtr), (N) * sizeof *(array)->items); \
            (array)->len += (N); \
            *(ok) = 1; \
        } else { \
            *(ok) = 0; \
        } \
    } while (0)

#define ResizeArray(array, N) \
    do { \
        bool ok; \
        ResizeArrayCheck(array, N, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define AppendArray(array, item)\
    do { \
        bool ok; \
        AppendArrayCheck(array, item, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define ExtendArray(array, manyPtr, N) \
    do { \
        bool ok; \
        ExtendArrayCheck(array, manyPtr, N, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define RemoveFromArray(array, index) \
    do { \
        if ((array)->len > 1 && (index) < (array)->len) { \
            for (uint32_t i = (index); i < (array)->len - 1; i++) { \
                (array)->items[i] = (array)->items[i + 1]; \
            } \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--; \
        } \
    } while (0)

#define FreeArray(array) ResizeArray(array, 0)

#if __STDC_VERSION__ >= 202300L || defined(__GNUC__)

#define AppendManyArray(array, ...) \
    do { \
        typeof(*(array)->items) tmp[] = {__VA_ARGS__}; \
        for (size_t i = 0; i < sizeof tmp / sizeof tmp[0]; i++) { \
            AppendArray(array, tmp[i]); \
        } \
    } while (0)

#endif

#define MISC_MAP_DEPTH_0    (0)
#define MISC_MAP_DEPTH_1    (1)
#define MISC_FNV_BASIS      (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME      (0x100000001b3ULL)
#define MISC_FNV_LIMIT      (32)
#define MISC_MAP_N_RESERVE  (32)
#define MISC_MAP_N_CONFLICT (MISC_MAP_N_RESERVE / 2)

typedef struct {
    const uint8_t *key;
    size_t len;
} MapKey;

typedef struct {
    MapKey key;
    void *value;
} MapEntry;

typedef Array(MapEntry) MapBucket;

typedef struct {
    Array(MapBucket) buckets;
    size_t failCount, failLimit;
} Map;

uint64_t  CreateFNVHash(const void *ptr, const size_t size);
int       VerifyFNVKey(const MapKey *lhs, const MapKey *rhs);

Map      *CreateMap(void);
bool      PutIntoMap(Map *map, const MapKey key, void *value, const size_t size);
MapEntry *GetEntryFromMap(Map *map, const MapKey key);
void     *GetFromMap(Map *map, const MapKey key);
bool      DeleteFromMap(Map *map, const MapKey key);
void      FreeMap(Map *map);

Arena    *CreateArena(size_t size, uint32_t flags, ...);
void     *ArenaAlloc(Arena *arena, size_t size, ...);
void     *ArenaRealloc(Arena *arena, void *ptr, size_t oldSize, size_t newSize, ...);
void      DestroyArena(Arena *arena);

#ifndef MISC_IMPL
#else

uint64_t CreateFNVHash(const void *ptr, const size_t size)
{
    const uint8_t *bytes = ptr;
    uint64_t baseNum = MISC_FNV_BASIS;

    if (size <= MISC_FNV_LIMIT) {
        for (size_t i = 0; i < size; i++) {
            baseNum *= MISC_FNV_PRIME;
            baseNum ^= bytes[i];
        }
    } else {
        for (size_t i = 0; i < MISC_FNV_LIMIT / 2; i++) {
            baseNum *= MISC_FNV_PRIME;
            baseNum ^= bytes[i];
        }
        for (size_t i = 0, j = size - 1; i < MISC_FNV_LIMIT / 2; i++, j--) {
            baseNum *= MISC_FNV_PRIME;
            baseNum ^= bytes[j];
        }
    }

    return baseNum;
}

int VerifyFNVKey(const MapKey *lhs, const MapKey *rhs)
{
    uint64_t hleft = CreateFNVHash(lhs->key, lhs->len);
    uint64_t hright = CreateFNVHash(rhs->key, rhs->len);
    size_t smallest = lhs->len > rhs->len ? rhs->len : lhs->len;
    int result = 0;

    if (hleft == hright && (result = memcmp(lhs, rhs, smallest)) == 0) {
        return 0;
    }
    return result;
}

int CompareEntry(const void *lhs, const void *rhs)
{
    return VerifyFNVKey(lhs, rhs);
}

void *FindOnBucket(MapBucket *bucket, const MapKey *signature)
{
    if (bucket->len <= MISC_ARRAY_RESERVE) {
        for (size_t i = 0; i < bucket->len; i++) {
            MapEntry *entry = &bucket->items[i];
            if (VerifyFNVKey(&entry->key, signature) == 0) {
                return entry;
            }
        }
        return NULL;
    } else {
        qsort(bucket->items, bucket->len, sizeof *bucket->items, CompareEntry);
        return bsearch(signature, bucket->items, bucket->len, sizeof *bucket->items, CompareEntry);
    }
}

Map *CreateMap(void)
{
    Map *map = calloc(1, sizeof *map);
    if (map == NULL) {
        return NULL;
    }

    map->failCount = 0;
    map->failLimit = MISC_MAP_N_CONFLICT;

    bool ok;
    ResizeArrayCheck(&map->buckets, MISC_MAP_N_RESERVE, &ok);
    if (!ok) {
        free(map);
        return NULL;
    }

    return map;
}

bool MakeSureMapIsFilled(Map *map)
{
    bool ok = true;
    if (map->buckets.cap < 1) {
        ResizeArrayCheck(&map->buckets, MISC_ARRAY_RESERVE, &ok);
    } else if (map->failCount >= map->failLimit) {
        ResizeArrayCheck(&map->buckets, map->buckets.cap * 2, &ok);
    }
    return ok;
}

bool PutIntoMap(Map *map, const MapKey key, void *value, const size_t size)
{
    if (map == NULL || key.key == NULL) { return false; }
    if (!MakeSureMapIsFilled(map))      { return false; }

    MapEntry entry = {
        .key = key,
        .value = value,
    };

    uint64_t index = CreateFNVHash(key.key, key.len) % map->buckets.cap;
    MapBucket *bucket = &map->buckets.items[index];
    bool ok = true;

    if (value != NULL && size > 1) {
        entry.value = malloc(size);
        if (entry.value == NULL) { return false; };
        memmove(entry.value, value, size);
    }

    if (bucket->len > 1) {
        map->failCount++;
    }

    AppendArrayCheck(bucket, entry, &ok);
    return ok;
}

MapBucket *GetBucketFromMap(Map *map, const MapKey key)
{
    if (map == NULL || key.key == NULL) { return NULL; }
    uint64_t index = CreateFNVHash(key.key, key.len) % map->buckets.cap;
    MapBucket *bucket = &map->buckets.items[index];
    switch (bucket->len) {
    case 0:
        return NULL;
    default:
        return bucket;
    }
}

MapEntry *GetEntryFromMap(Map *map, const MapKey key)
{
    MapBucket *bucket = GetBucketFromMap(map, key);
    if (bucket == NULL) return NULL;

    switch (bucket->len) {
    case 1:
        return &bucket->items[0];
    default:
        return FindOnBucket(bucket, &key);
    }
    // if (map == NULL || key.key == NULL) { return NULL; }
    // uint64_t index = CreateFNVHash(key.key, key.len) % map->buckets.cap;
    // MapBucket *bucket = &map->buckets.items[index];
    // switch (bucket->len) {
    // case 0:
    //     return NULL;
    // case 1:
    //     return &bucket->items[0];
    // default:
    //     return FindOnBucket(bucket, &key);
    // }
}

void *GetFromMap(Map *map, const MapKey key)
{
    MapEntry *entry = GetEntryFromMap(map, key);
    if (entry == NULL) return NULL;
    return entry->value;
}


bool DeleteFromMap(Map *map, const MapKey key)
{
    MapBucket *bucket = GetBucketFromMap(map, key);
    if (bucket == NULL) return false;

    MapEntry *target = GetEntryFromMap(map, key);
    if (target == NULL) return false;

    if (bucket->len < 2) {
        FreeArray(bucket);
        return true;
    }

    for (uint32_t i = 0; i < bucket->len; i++) {
        if (VerifyFNVKey(&bucket->items[i].key, &key) == 0) {
            RemoveFromArray(bucket, i);
            printf("%u\n", i);
            return true;
        }
    }

    return false;
}
void FreeMap(Map *map)
{
    if (map != NULL) {
        for (uint32_t i = 0; i < map->buckets.len; i++) {
            if (map->buckets.items[i].cap != 0)
                FreeArray(&map->buckets.items[i]);
        }
        FreeArray(&map->buckets);
        map->failCount = 0;
        map->failLimit = 0;
        free(map);
    }
}

/* Arena: linear allocator.
 * this data structure is usually used to reduce
 * the call of malloc/realloc by preallocating
 * some amount of bytes into it's buffer, and simply
 * return the incremented address from that buffer
 * by its offset.
 *
 * The flags is used to specify what kind of arena is it.
 * Flags:
 * - MISC_ARDEFAULT: / MISC_ARHEAP: When this used, the buffer
 *   capacity is exactly `n` bytes, and it uses malloc/realloc
 *   to do that.
 * - MISC_ARSTACK: Use stack buffer at exactly `n` - sizeof Arena,
 *   so the size should be atleast sizeof Arena + 1, if it isn't,
 *   return NULL. The buffer is passed as third argument.
 * - MISC_ARNOGROW: If this flag is set, the arena doesn't grow
 *   exponentially, if the buffer is full, it cannot allocate anymore
 *   and simply return NULL.
 *
 * If MISC_ARSTACK: is specified but not MISC_ARNOGROW: , the user MUST
 * provide additional buffer in the third argument.
 * */

Arena *CreateArena(size_t size, uint32_t flags, ...)
{
    Arena *headNode = NULL;
    va_list va;
    va_start(va, flags);

    if (flags & MISC_ARSTACK && size < sizeof *headNode + 1) {
        return NULL;
    }

    switch (flags) {
    case MISC_ARDEFAULT:
    case MISC_ARDEFAULT | MISC_ARNOGROW:
        headNode = malloc(sizeof *headNode + size);
        break;
    case MISC_ARSTACK:
    case MISC_ARSTACK | MISC_ARNOGROW:
        headNode = va_arg(va, void *);
        break;
    default:
        goto none;
    }

    if (!headNode) {
        goto none;
    }

    headNode->next = NULL;
    headNode->total = flags & MISC_ARSTACK ? size - sizeof *headNode : size;
    headNode->offset = 0;
    headNode->flags = flags;

none:
    va_end(va);
    return headNode;
}

static Arena *FindCapableArena(Arena *arena, size_t size, int *onFound)
{
    Arena *visitor = arena, *lastNonnull = NULL;
    if (visitor->flags & MISC_ARNOGROW) {
        if (RemainsOfArena(arena) >= size) {
            *onFound = 1;
        } else {
            *onFound = 0;
        }
        return visitor;
    }

    while (visitor) {
        if (RemainsOfArena(visitor) >= size) {
            *onFound = 1;
            return visitor;
        }
        lastNonnull = visitor;
        visitor = visitor->next;
    }

    *onFound = 0;
    return lastNonnull;
}

void *ArenaAlloc(Arena *arena, size_t size, ...)
{
    Arena *suitable;
    va_list va;
    int found = 0;
    void *result = NULL;

    if (arena == NULL || size == 0)
        return NULL;

    va_start(va, size);
    if (size > RemainsOfArena(arena)) {
        suitable = FindCapableArena(arena, size, &found);
    } else {
        suitable = arena, found = 1;
    }

    if (!found) {
        void *optional = NULL;
        if (suitable->flags & MISC_ARNOGROW) {
            goto none;
        }

        if (suitable->flags & MISC_ARSTACK) {
            optional = va_arg(va, void *);
        }

        suitable->next = CreateArena(size + suitable->total, suitable->flags, optional);
        if (!suitable->next) {
            goto none;
        }

        suitable = suitable->next;
    }

    uint8_t *offsetPtr = (uint8_t *)MISC_VOIDPTR(suitable) + sizeof *suitable;
    result = offsetPtr + suitable->offset;
    suitable->offset = suitable->offset + size;

none:
    va_end(va);
    return result;
}

void *ArenaRealloc(Arena *arena, void *ptr, size_t oldSize, size_t newSize, ...)
{
    void *optional = NULL;
    void *result = NULL;
    va_list va;

    if (!arena) {
        goto none;
    }

    va_start(va, newSize);
    if (arena->flags & MISC_ARSTACK) {
        optional = va_arg(va, void *);
    }

    result = ArenaAlloc(arena, newSize, optional);
    if (result == NULL) {
        goto none;
    } else if (ptr == NULL) {
        goto none;
    }

    memmove(result, ptr, oldSize > newSize ? newSize : oldSize);
none:
    va_end(va);
    return result;
}

void DestroyArena(Arena *arena)
{
    while (arena) {
        Arena *tmp = arena->next;
        if (arena->flags & MISC_ARHEAP) {
            free(arena);
        } else {
            memset(arena, 0, arena->total + sizeof *arena);
        }

        arena = tmp;
    }
}

#endif

#endif
