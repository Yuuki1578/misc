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

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef size_t usize;

#define MISC_VOIDPTR(expr) ((void*)(expr))
#define MISC_ARENA_PAGESIZE (1ULL << 12ULL)
#define MISC_ARSTACK (0x1)
#define MISC_ARHEAP (0x10)
#define MISC_ARNOGROW (0x100)
#define MISC_ARDEFAULT (MISC_ARHEAP)

typedef struct Arena Arena;
struct Arena {
    Arena* next;
    u32 total, offset, flags;
};

#define arena_remains(a) ((a)->total - (a)->offset)
Arena* arena_init(u64 size, u32 flags, ...);
void* arena_alloc(Arena* arena, u64 size, ...);
void* arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, ...);
void arena_free(Arena* arena);

#ifdef MISC_IMPL

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

Arena* arena_init(u64 size, u32 flags, ...)
{
    Arena* head_node = NULL;
    va_list va;
    va_start(va, flags);

    if (flags & MISC_ARSTACK && size < sizeof *head_node + 1)
        return NULL;

    switch (flags) {
    case MISC_ARDEFAULT:
    case MISC_ARDEFAULT | MISC_ARNOGROW:
        head_node = malloc(sizeof *head_node + size);
        break;
    case MISC_ARSTACK:
    case MISC_ARSTACK | MISC_ARNOGROW:
        head_node = va_arg(va, void*);
        break;
    default:
        goto none;
    }

    if (head_node == NULL) goto none;

    head_node->next = NULL;
    head_node->total = flags & MISC_ARSTACK ? size - sizeof *head_node : size;
    head_node->offset = 0;
    head_node->flags = flags;

none:
    va_end(va);
    return head_node;
}

static Arena* arena_find_exact(Arena* arena, u64 size, int* found)
{
    Arena *visitor = arena, *last_nonnull = NULL;
    if (visitor->flags & MISC_ARNOGROW) {
        if (arena_remains(arena) >= size)
            *found = 1;
        else
            *found = 0;
        return visitor;
    }

    while (visitor) {
        if (arena_remains(visitor) >= size) {
            *found = 1;
            return visitor;
        }
        last_nonnull = visitor;
        visitor = visitor->next;
    }

    *found = 0;
    return last_nonnull;
}

void* arena_alloc(Arena* arena, u64 size, ...)
{
    Arena* suitable;
    va_list va;
    int found = 0;
    void* result = NULL;

    if (arena == NULL || size == 0) return NULL;

    va_start(va, size);
    if (size > arena_remains(arena)) {
        suitable = arena_find_exact(arena, size, &found);
    } else {
        suitable = arena, found = 1;
    }

    if (!found) {
        void* optional = NULL;
        if (suitable->flags & MISC_ARNOGROW) goto none;

        if (suitable->flags & MISC_ARSTACK)
            optional = va_arg(va, void*);

        suitable->next = arena_init(size + suitable->total, suitable->flags, optional);
        if (suitable->next == NULL) goto none;

        suitable = suitable->next;
    }

    u8* offset_ptr = (u8*)MISC_VOIDPTR(suitable) + sizeof *suitable;
    result = offset_ptr + suitable->offset;
    suitable->offset = suitable->offset + size;

none:
    va_end(va);
    return result;
}

void* arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, ...)
{
    void* optional = NULL;
    void* result = NULL;
    va_list va;

    if (arena == NULL) goto none;

    va_start(va, new_size);
    if (arena->flags & MISC_ARSTACK)
        optional = va_arg(va, void*);

    result = arena_alloc(arena, new_size, optional);
    if (result == NULL)
        goto none;
    else if (ptr == NULL)
        goto none;

    memmove(result, ptr, old_size > new_size ? new_size : old_size);
none:
    va_end(va);
    return result;
}

void arena_free(Arena* arena)
{
    while (arena) {
        Arena* tmp = arena->next;
        if (arena->flags & MISC_ARHEAP)
            free(arena);
        else
            memset(arena, 0, arena->total + sizeof *arena);

        arena = tmp;
    }
}

#endif

#define MISC_ARRAY_RESERVE (8)

#define Array(T)  \
    struct {      \
        T* items; \
        u64 cap;  \
        u64 len;  \
    }

#define array_is_empty(array) ((array) != NULL ? ((array)->items == NULL && !(array)->cap) : 1)
#define array_remains(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define array_try_resize(array, N, ok)                                       \
    do {                                                                     \
        if ((N) <= 0) {                                                      \
            free((array)->items);                                            \
            (array)->items = NULL;                                           \
            (array)->cap = 0;                                                \
            (array)->len = 0;                                                \
            *(ok) = 1;                                                       \
        } else {                                                             \
            void* tmp;                                                       \
            if ((array)->items == NULL) {                                    \
                tmp = calloc((N), sizeof *(array)->items);                   \
            } else {                                                         \
                tmp = realloc((array)->items, (N) * sizeof *(array)->items); \
            }                                                                \
            if (tmp != NULL) {                                               \
                *(ok) = 1;                                                   \
                (array)->items = tmp;                                        \
                (array)->cap = (N);                                          \
                if ((N) < (array)->len) {                                    \
                    (array)->len = (N);                                      \
                }                                                            \
            } else {                                                         \
                *(ok) = 0;                                                   \
            }                                                                \
        }                                                                    \
    } while (0)

#define array_try_append(array, item, ok)                                   \
    do {                                                                    \
        if ((array)->cap <= (array)->len) {                                 \
            array_try_resize(array, (array)->cap + MISC_ARRAY_RESERVE, ok); \
        }                                                                   \
        if (*(ok)) {                                                        \
            (array)->items[(array)->len++] = (item);                        \
        }                                                                   \
    } while (0)

#define array_try_extend(array, many_ptr, N, ok)                                              \
    do {                                                                                      \
        if ((many_ptr) != NULL && (N) > 0) {                                                  \
            if (array_is_empty(array) || array_remains(array) <= (N)) {                       \
                array_try_resize(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok);         \
                if (!*(ok)) {                                                                 \
                    break;                                                                    \
                }                                                                             \
            }                                                                                 \
            memmove((array)->items + (array)->len, (many_ptr), (N) * sizeof *(array)->items); \
            (array)->len += (N);                                                              \
            *(ok) = 1;                                                                        \
        } else {                                                                              \
            *(ok) = 0;                                                                        \
        }                                                                                     \
    } while (0)

#define array_resize(array, N)           \
    do {                                 \
        bool ok;                         \
        array_try_resize(array, N, &ok); \
        if (!ok) {                       \
            abort();                     \
        }                                \
    } while (0)

#define array_append(array, item)           \
    do {                                    \
        bool ok;                            \
        array_try_append(array, item, &ok); \
        if (!ok) {                          \
            abort();                        \
        }                                   \
    } while (0)

#define array_extend(array, many_ptr, N)           \
    do {                                           \
        bool ok;                                   \
        array_try_extend(array, many_ptr, N, &ok); \
        if (!ok) {                                 \
            abort();                               \
        }                                          \
    } while (0)

#define array_remove_at(array, index)                                             \
    do {                                                                          \
        if ((array)->len > 1 && (index) < (array)->len) {                         \
            for (u64 i = (index); i < (array)->len - 1; i++) {                    \
                (array)->items[i] = (array)->items[i + 1];                        \
            }                                                                     \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--;                                                       \
        }                                                                         \
    } while (0)

#define array_free(array) array_resize(array, 0)

#define Slice(T)        \
    struct {            \
        const T* items; \
        u64 len;        \
    }

typedef Array(char) String;
typedef Slice(char) StringRef;

// Exclusive
#define slice_from(slice, ptr, length, begin, end)    \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        u64 _b, _e;                                 \
        _b = (begin) > (length) ? (length) : (begin); \
        _e = (end) > (length) ? (length) : (end);     \
        (slice)->items = (ptr) + (_b);                \
        (slice)->len = ((_e) - (_b));                 \
    } while (0)

#define slice_from_array(slice, array, begin, end) slice_from(slice, (array)->items, (array)->len, begin, end)

/*
Legends:
    function with prefixes cstring_* is going to use traditional char*
    function with prefixes string_* is going to use String
    function with prefixes stringref_* is going to use StringRef
*/

#define string_fmt(s) (int)(s).len, (s).items
StringRef stringref_from(const char* cstr, u64 begin, u64 end);
StringRef stringref_from_string(String* str, u64 begin, u64 end);
String string_printf(const char* fmt, ...);
char* cstring_printf(Arena* allocator, const char* fmt, ...);

#ifdef MISC_IMPL

StringRef stringref_from(const char* cstr, u64 begin, u64 end)
{
    StringRef ref = {0};
    if (cstr == NULL || end < begin)
        return ref;

    u64 len = strlen(cstr);
    slice_from(&ref, cstr, len, begin, end);
    return ref;
}

StringRef stringref_from_string(String* str, u64 begin, u64 end)
{
    StringRef ref = {0};
    slice_from_array(&ref, str, begin, end);
    return ref;
}

char* cstring_printf(Arena* allocator, const char* fmt, ...)
{
    char* buf = NULL;
    va_list va;

    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        if ((buf = arena_alloc(allocator, (u64)size + 1)) == NULL)
            goto end;

        va_start(va, fmt);
        vsnprintf(buf, (u64)size + 1, fmt, va);
        va_end(va);
    }

end:;
    return buf;
}

String string_printf(const char* fmt, ...)
{
    String str = {0};
    va_list va;
    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        bool ok;
        array_try_resize(&str, (u64)size + 1, &ok);
        if (!ok)
            return str;

        va_start(va, fmt);
        size = vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size > 0 ? size : 0;
    }

    return str;
}

#endif

#define MISC_FNV_BASIS (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME (0x100000001b3ULL)

#ifndef MISC_HASHMAP_LOADFACTOR
#define MISC_HASHMAP_LOADFACTOR (0.85)
#else
#if MISC_HASHMAP_LOADFACTOR >= 1.0
#error Load factor must be less than 1.0
#endif
#endif

#ifndef MISC_HASHMAP_INITCAP
#define MISC_HASHMAP_INITCAP (8)
#endif

typedef struct {
    void* key;
    u64 len;
} HashKey;

typedef struct ChainEntry {
    HashKey key;
    void* value;
    u64 hash;
    struct ChainEntry* next;
} ChainEntry;

typedef Array(ChainEntry) ChainTable;

typedef struct {
    ChainTable table;
} ChainMap;

#define chainmap_load_factor(map) ((double)(map)->table.len / (double)(map)->table.cap)
#define chainmap_put_cstr(map, cstr, value, size) chainmap_put(map, (HashKey) { .key = (void*)(cstr), .len = strlen(cstr) }, value, size)
#define chainmap_get_cstr(map, cstr) chainmap_get(map, (HashKey) { .key = (void*)(cstr), .len = strlen(cstr) })

u64 fnv_init(const void* ptr, u64 size);
bool chainmap_put(ChainMap* map, HashKey key, void* value, u64 size);
void* chainmap_get(ChainMap* map, HashKey key);
void chainmap_delete_at(ChainMap* map, HashKey key);
void chainmap_free(ChainMap* map);

#define make_opaque(T, ...) ((void*)&(T){__VA_ARGS__})

// map == struct { K key; V value; }**
// K   != char*
// V   == any
#define map_put(map, K, V) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        (*(map))->value = (V); \
        HashKey __K = { \
            .key = &(*(map))->key, \
            .len = sizeof (*(map))->key, \
        }; \
        void* __V = &(*(map))->value; \
        usize __Vsz = sizeof &(*(map))->value; \
        chainmap_put(real, __K, __V, __Vsz); \
    } while (0)

// map == struct { K key; V value; }**
// K   == char*
// V   == any
#define mapstr_put(map, K, length, V) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        (*(map))->value = (V); \
        HashKey __K = { \
            .key = (*(map))->key, \
            .len = (length), \
        }; \
        void* __V = &(*(map))->value; \
        usize __Vsz = sizeof &(*(map))->value; \
        chainmap_put(real, __K, __V, __Vsz); \
    } while (0)

// map    == struct { K key; V value; }**
// K      != char*
// stored == V**
#define map_get(map, K, stored) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        HashKey __K = { \
            .key = &(*(map))->key, \
            .len = sizeof (*(map))->key, \
        }; \
        void* value = chainmap_get(real, __K); \
        *(stored) = value; \
    } while (0)

// map    == struct { K key; V value; }**
// K      == char*
// stored == V**
#define mapstr_get(map, K, length, stored) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        HashKey __K = { \
            .key = (*(map))->key, \
            .len = (length), \
        }; \
        void* value = chainmap_get(real, __K); \
        *(stored) = value; \
    } while (0)

// map == struct { K key; V value; }**
// K   != char*
// V   == any
#define map_delete_at(map, K) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        HashKey __K = { \
            .key = &(*(map))->key, \
            .len = sizeof (*(map))->key, \
        }; \
        chainmap_delete_at(real, __K); \
    } while (0)

// map == struct { K key; V value; }**
// K   == any
// V   == any
#define mapstr_delete_at(map, K, length) \
    do { \
        if (*(map) == NULL) { \
            *(map) = malloc(sizeof(ChainMap) + sizeof **(map)); \
            assert(*(map) != NULL); \
        } \
        ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
        (*(map))->key = (K); \
        HashKey __K = { \
            .key = (*(map))->key, \
            .len = (length), \
        }; \
        chainmap_delete_at(real, __K); \
    } while (0)

#define map_free(map) \
    do { \
        if (*(map) != NULL) { \
            ChainMap* real = (void*)((u8*)(void*)*(map) + sizeof **(map)); \
            chainmap_free(real); \
            free(*(map)); \
            *(map) = NULL; \
        } \
    } while (0)

#ifdef MISC_IMPL

u64 fnv_init(const void* ptr, u64 size)
{
    const u8* bytes = ptr;
    u64 base_number = MISC_FNV_BASIS;
    for (u64 i = 0; i < size; i++) {
        base_number *= MISC_FNV_PRIME;
        base_number ^= bytes[i];
    }
    return base_number;
}

static ChainEntry* chainentry_last(ChainEntry* entry)
{
    while (entry != NULL) {
        if (entry->next == NULL) return entry;
        entry = entry->next;
    }
    return NULL;
}

static ChainEntry* chainentry_find(ChainEntry* entry, HashKey key, u64 hash)
{
    while (entry != NULL) {
        if (entry->hash == hash && memcmp(entry->key.key, key.key, key.len) == 0)
            return entry;

        entry = entry->next;
    }

    return NULL;
}

static bool chainmap_rehash(ChainMap* map)
{
    ChainTable newer = {0};
    u64 new_cap = map->table.cap * 2;
    bool ok;

    array_try_resize(&newer, new_cap, &ok);
    if (!ok) return false;
    newer.len = map->table.len;

    for (u64 i = 0; i < map->table.cap; i++) {
        ChainEntry* curr = &map->table.items[i];
        ChainEntry* head = curr;
        if (curr->hash == 0) continue;

        while (curr != NULL) {
            ChainEntry* next = curr->next;
            u64 index = curr->hash % newer.cap;
            ChainEntry* new_slot = &newer.items[index];

            if (new_slot->hash == 0) {
                *new_slot = *curr;
                new_slot->next = NULL;
            } else {
                ChainEntry* tail = chainentry_last(new_slot);
                tail->next = malloc(sizeof(ChainEntry));
                if (tail->next == NULL)
                    continue;

                *tail->next = *curr;
                tail->next->next = NULL;
            }

            if (curr != head) free(curr);
            curr = next;
        }
    }

    array_free(&map->table);
    map->table = newer;
    return ok;
}

static bool chainmap_try_init(ChainMap* map)
{
    bool ok = true;
    if (map->table.cap < MISC_HASHMAP_INITCAP)
        array_try_resize(&map->table, MISC_HASHMAP_INITCAP, &ok);
    if (chainmap_load_factor(map) >= MISC_HASHMAP_LOADFACTOR)
        ok = chainmap_rehash(map);

    return true;
}

static bool chainentry_init(ChainEntry* entry, HashKey key, u64 hash, void* value, u64 size)
{
    u8* pool = malloc(key.len + size);
    if (pool == NULL) return false;

    entry->key.key = (void*)(pool);
    entry->value = (void*)(pool + key.len);
    memmove(entry->key.key, key.key, key.len);
    memmove(entry->value, value, size);
    entry->key.len = key.len;
    entry->hash = hash;
    entry->next = NULL;
    return true;
}

bool chainmap_put(ChainMap* map, HashKey key, void* value, u64 size)
{
    if (key.key == NULL || value == NULL) return false;;
    if (!chainmap_try_init(map)) return false;

    u64 hash = fnv_init(key.key, key.len);
    u64 index = hash % map->table.cap;
    ChainEntry* entry = &map->table.items[index];
    bool ok = true;

    if (entry->hash == 0) {
        ok = chainentry_init(entry, key, hash, value, size);
    } else {
        ChainEntry* tail = chainentry_last(entry);
        tail->next = malloc(sizeof(ChainEntry));
        if (tail->next == NULL) return false;
        ok = chainentry_init(tail->next, key, hash, value, size);
    }

    if (ok) map->table.len++;
    return ok;
}

static ChainEntry* chainmap_get_entry(ChainMap* map, HashKey key)
{
    u64 hash = fnv_init(key.key, key.len);
    u64 index = hash % map->table.cap;
    ChainEntry* entry = &map->table.items[index];
    return chainentry_find(entry, key, hash);
}

void* chainmap_get(ChainMap* map, HashKey key)
{
    ChainEntry* entry = chainmap_get_entry(map, key);
    if (entry != NULL) return entry->value;
    else return NULL;
}

void chainmap_delete_at(ChainMap* map, HashKey key)
{
    ChainEntry* entry = chainmap_get_entry(map, key);
    if (entry == NULL || entry->hash == 0) return;

    ChainEntry* child = entry->next;
    free(entry->key.key);
    memset(entry, 0, sizeof *entry);
    entry->next = child;
}

void chainmap_free(ChainMap* map)
{
    for (u64 i = 0; i < map->table.cap; i++) {
        ChainEntry* curr = &map->table.items[i];
        ChainEntry* head = curr;
        if (curr->hash == 0) continue;

        while (curr != NULL) {
            ChainEntry* next = curr->next;
            if (curr->key.key != NULL) free(curr->key.key);
            if (curr != head) free(curr);
            curr = next;
        }
    }
    array_free(&map->table);
}

#endif

#endif
