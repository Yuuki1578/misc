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

#define MISC_VOIDPTR(expr)  ((void *)(expr))
#define MISC_ARENA_PAGESIZE (1ULL << 12ULL)
#define MISC_ARSTACK        (0x1)
#define MISC_ARHEAP         (0x10)
#define MISC_ARNOGROW       (0x100)
#define MISC_ARDEFAULT      (MISC_ARHEAP)

typedef struct arena arena_t;
struct arena {
    arena_t *next;
    uint32_t total, offset, flags;
};

#define  arena_remains(a) ((a)->total - (a)->offset)
arena_t *arena_init(size_t size, uint32_t flags, ...);
void    *arena_alloc(arena_t *arena, size_t size, ...);
void    *arena_realloc(arena_t *arena, void *ptr, size_t old_size, size_t new_size, ...);
void     arena_free(arena_t *arena);

#define MISC_ARRAY_RESERVE (8)

#define array_t(type) \
    struct { \
        type *items; \
        uint32_t cap; \
        uint32_t len; \
    }

#define array_is_empty(array) ((array) != NULL ? ((array)->items == NULL && !(array)->cap) : 0)
#define array_remains(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define array_try_resize(array, N, ok) \
    do { \
        if ((N) <= 0) { \
            free((array)->items); \
            (array)->items = NULL; \
            (array)->cap = 0; \
            (array)->len = 0; \
            *(ok) = 1; \
        } else { \
            void *tmp; \
            if ((array)->items == NULL) { \
                tmp = calloc((N), sizeof *(array)->items); \
            } else { \
                tmp = realloc((array)->items, (N) * sizeof *(array)->items); \
            } \
            if (tmp != NULL) { \
                *(ok) = 1; \
                (array)->items = tmp; \
                (array)->cap = (N); \
                if ((N) < (array)->len) { \
                    (array)->len = (N); \
                } \
            } else { \
                *(ok) = 0; \
            } \
        } \
    } while (0)

#define array_try_append(array, item, ok) \
    do { \
        if ((array)->cap <= (array)->len) { \
            array_try_resize(array, (array)->cap + MISC_ARRAY_RESERVE, ok); \
        } \
        if (*(ok)) { \
            (array)->items[(array)->len++] = (item); \
        } \
    } while (0)

#define array_try_extend(array, many_ptr, N, ok) \
    do { \
        if ((many_ptr) != NULL && (N) > 0) { \
            if (array_is_empty(array) || array_remains(array) <= (N)) { \
                array_try_resize(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok); \
                if (!*(ok)) { \
                    break; \
                } \
            } \
            memmove((array)->items + (array)->len, (many_ptr), (N) * sizeof *(array)->items); \
            (array)->len += (N); \
            *(ok) = 1; \
        } else { \
            *(ok) = 0; \
        } \
    } while (0)

#define array_resize(array, N) \
    do { \
        bool ok; \
        array_try_resize(array, N, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define array_append(array, item)\
    do { \
        bool ok; \
        array_try_append(array, item, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define array_extend(array, many_ptr, N) \
    do { \
        bool ok; \
        array_try_extend(array, many_ptr, N, &ok); \
        if (!ok) { \
            abort(); \
        } \
    } while (0)

#define array_remove_at(array, index) \
    do { \
        if ((array)->len > 1 && (index) < (array)->len) { \
            for (uint32_t i = (index); i < (array)->len - 1; i++) { \
                (array)->items[i] = (array)->items[i + 1]; \
            } \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--; \
        } \
    } while (0)

#define array_free(array) array_resize(array, 0)

#define MISC_FNV_BASIS         (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME         (0x100000001b3ULL)
#define MISC_FNV_LIMIT         (64)
#define MISC_HASHMAP_INITCAP   (16)
#define MISC_HASHMAP_THRESHOLD (MISC_HASHMAP_INITCAP * 2)

typedef struct {
    uint64_t hash;
    uint8_t *key;
    size_t len;
} hashkey_t;

typedef struct hashentry {
    hashkey_t key;
    void *value;
    struct hashentry *next;
} hashentry_t;

typedef struct {
    array_t(hashentry_t) table;
    uint32_t collide, threshold;
    // TODO: Add load factor for better performance tracking
} hashmap_t;

uint64_t     fnv_init(const void *ptr, const size_t size);
int          fnv_memcmp(const void *left, const size_t left_len, const void *right, const size_t right_len);
bool         hashmap_put(hashmap_t *map, hashkey_t key, void *value, const size_t size);
hashentry_t *hashmap_get_entry(const hashmap_t *map, const hashkey_t key);
void        *hashmap_get(const hashmap_t *map, const hashkey_t key);
bool         hashmap_delete_at(hashmap_t *map, const hashkey_t key); // TODO
void         hashmap_free(hashmap_t *map);

#ifdef MISC_IMPL
uint64_t fnv_init(const void *ptr, const size_t size)
{
    const uint8_t *bytes = ptr;
    uint64_t base_number = MISC_FNV_BASIS;

    if (size <= MISC_FNV_LIMIT) {
        for (size_t i = 0; i < size; i++) {
            base_number *= MISC_FNV_PRIME;
            base_number ^= bytes[i];
        }
    } else {
        for (size_t i = 0; i < MISC_FNV_LIMIT / 2; i++) {
            base_number *= MISC_FNV_PRIME;
            base_number ^= bytes[i];
        }
        for (size_t i = 0, j = size - 1; i < MISC_FNV_LIMIT / 2; i++, j--) {
            base_number *= MISC_FNV_PRIME;
            base_number ^= bytes[j];
        }
    }

    return base_number;
}

int fnv_memcmp(const void *left, const size_t left_len, const void *right, const size_t right_len)
{
    size_t smallest = left_len > right_len ? right_len : left_len;
    int result;

    if ((result = memcmp(left, right, smallest)) == 0)
        return 0;

    return result;
}

bool hashentry_is_empty(const hashentry_t *entry)
{
    hashentry_t zeroed = { 0 };
    return memcmp(entry, &zeroed, sizeof zeroed) == 0;
}

hashentry_t *hashentry_find_tail(hashentry_t *head)
{
    if (head == NULL) return NULL;
    hashentry_t *current = head;
    while (current->next != NULL) current = current->next;
    return current;
}

// HELL
void hashmap_rehash(hashmap_t* map)
{
    // TODO rehash all invalid hash value
    array_t(hashentry_t) *table = (void*) &map->table;

    for (size_t i = 0; i < table->cap; i++) {
        hashentry_t *entry = &table->items[i];
        if (hashentry_is_empty(entry)) continue;

        uint64_t new_index = entry->key.hash % table->cap;
        hashentry_t *new_entry = &table->items[new_index];

        if (hashentry_is_empty(new_entry)) {
            *new_entry = *entry;
        } else {
            hashentry_t *tail = hashentry_find_tail(new_entry);
            /* this happen when it supposed to be null, like for fuck sake
             * the allocator API is not zeroing it out dude
             * */
            if (tail->next != NULL) abort();
            tail->next = entry;
        }
        memset(entry, 0, sizeof *entry);
    }
}

bool hashmap_try_reserve(hashmap_t *map)
{
    array_t(hashentry_t) *table = (void*) &map->table;
    bool ok;

    if (table->cap < MISC_HASHMAP_INITCAP) {
        array_try_resize(table, MISC_HASHMAP_INITCAP, &ok);
        if (!ok) return false;
    }
    if (map->collide >= map->threshold) {
        array_try_resize(table, table->cap * 2, &ok);
        if (!ok) return false;
        hashmap_rehash(map);
        map->threshold *= 2;
    }
    return true;
}

hashentry_t *hashentry_init(hashkey_t key, void *value)
{
    hashentry_t *entry = malloc(sizeof *entry);
    if (entry == NULL) return NULL;
    memset(entry, 0, sizeof *entry);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

hashentry_t *hashentry_find_exact(hashentry_t *head, const hashkey_t key)
{
    uint64_t hash = fnv_init(key.key, key.len);
    while (head != NULL) {
        size_t smallest = head->key.len > key.len ? key.len : head->key.len;
        if (head->key.hash == hash && memcmp(head->key.key, key.key, smallest) == 0)
            return head;

        head = head->next;
    }
    return NULL;
}

bool hashentry_is_head(hashentry_t *maybe_head)
{
    return maybe_head->next == NULL;
}

bool hashmap_put(hashmap_t *map, hashkey_t key, void *value, const size_t size)
{
    if (map->threshold < MISC_HASHMAP_THRESHOLD)
        map->threshold = MISC_HASHMAP_THRESHOLD;

    if (!hashmap_try_reserve(map)) return false;

    array_t(hashentry_t) *table = (void*) &map->table;
    key.hash = fnv_init(key.key, key.len);
    uint64_t index = key.hash % table->cap;
    hashentry_t *entry = &table->items[index];
    hashentry_t appended = {
        .key = key,
        .value = value,
        .next = NULL,
    };
    appended.key.key = malloc(key.len);
    if (appended.key.key == NULL) return false;
    memmove(appended.key.key, key.key, key.len);


    if (value != NULL && size > 0) {
        appended.value = malloc(size);
        if (appended.value == NULL) return false;
        memmove(appended.value, value, size);
    }

    // Jackpot
    if (hashentry_is_empty(entry)) {
        *entry = appended;

    // Not so lucky
    } else {
        hashentry_t *tail = hashentry_find_tail(entry);
        if (tail == NULL) return false;

        hashentry_t *end = hashentry_init(key, appended.value);
        if (end == NULL) return false;

        tail->next = end;
        map->collide++;
    }
    return true;
}

hashentry_t *hashmap_get_entry(const hashmap_t *map, const hashkey_t key)
{
    array_t(hashentry_t) *table = (void*) &map->table;
    uint64_t hash = fnv_init(key.key, key.len);
    uint64_t index = hash % table->cap;
    hashentry_t *entry = &table->items[index];
    
    if (hashentry_is_empty(entry)) return NULL;
    if (hashentry_is_head(entry)) return entry; /* Jackpot */
    return hashentry_find_exact(entry, key);
}

void *hashmap_get(const hashmap_t *map, const hashkey_t key)
{
    hashentry_t *entry = hashmap_get_entry(map, key);
    if (entry != NULL) return entry->value;
    return NULL;
}

void hashmap_free(hashmap_t *map)
{
    for (size_t i = 0; i < map->table.cap; i++) {
        hashentry_t *entry = &map->table.items[i];
        if (hashentry_is_empty(entry)) continue;

        hashentry_t
            *node = entry,
            *next = NULL;

        while (node != NULL) {
            if (node->value != NULL) free(node->value);
            if (node->key.key != NULL) free(node->key.key);
            next = node->next;

            /* if node == entry, which is the head, skip free
             * because it's managed by array_* API
             * */
            if (node != entry) free(node); /* Not head */
            node = next;
        }
    }
    array_free(&map->table);
    map->collide = 0, map->threshold = 0;
}

/* arena_t: linear allocator.
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
 * - MISC_ARSTACK: Use stack buffer at exactly `n` - sizeof arena_t,
 *   so the size should be atleast sizeof arena_t + 1, if it isn't,
 *   return NULL. The buffer is passed as third argument.
 * - MISC_ARNOGROW: If this flag is set, the arena doesn't grow
 *   exponentially, if the buffer is full, it cannot allocate anymore
 *   and simply return NULL.
 *
 * If MISC_ARSTACK: is specified but not MISC_ARNOGROW: , the user MUST
 * provide additional buffer in the third argument.
 * */

arena_t *arena_init(size_t size, uint32_t flags, ...)
{
    arena_t *head_node = NULL;
    va_list va;
    va_start(va, flags);

    if (flags & MISC_ARSTACK && size < sizeof *head_node + 1) {
        return NULL;
    }

    switch (flags) {
    case MISC_ARDEFAULT:
    case MISC_ARDEFAULT | MISC_ARNOGROW:
        head_node = malloc(sizeof *head_node + size);
        break;
    case MISC_ARSTACK:
    case MISC_ARSTACK | MISC_ARNOGROW:
        head_node = va_arg(va, void *);
        break;
    default:
        goto none;
    }

    if (!head_node) {
        goto none;
    }

    head_node->next = NULL;
    head_node->total = flags & MISC_ARSTACK ? size - sizeof *head_node : size;
    head_node->offset = 0;
    head_node->flags = flags;

none:
    va_end(va);
    return head_node;
}

static arena_t *arena_find_exact(arena_t *arena, size_t size, int *found)
{
    arena_t *visitor = arena, *last_nonnull = NULL;
    if (visitor->flags & MISC_ARNOGROW) {
        if (arena_remains(arena) >= size) {
            *found = 1;
        } else {
            *found = 0;
        }
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

void *arena_alloc(arena_t *arena, size_t size, ...)
{
    arena_t *suitable;
    va_list va;
    int found = 0;
    void *result = NULL;

    if (arena == NULL || size == 0)
        return NULL;

    va_start(va, size);
    if (size > arena_remains(arena)) {
        suitable = arena_find_exact(arena, size, &found);
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

        suitable->next = arena_init(size + suitable->total, suitable->flags, optional);
        if (!suitable->next) {
            goto none;
        }

        suitable = suitable->next;
    }

    uint8_t *offset_ptr = (uint8_t *)MISC_VOIDPTR(suitable) + sizeof *suitable;
    result = offset_ptr + suitable->offset;
    suitable->offset = suitable->offset + size;

none:
    va_end(va);
    return result;
}

void *arena_realloc(arena_t *arena, void *ptr, size_t old_size, size_t new_size, ...)
{
    void *optional = NULL;
    void *result = NULL;
    va_list va;

    if (!arena) {
        goto none;
    }

    va_start(va, new_size);
    if (arena->flags & MISC_ARSTACK) {
        optional = va_arg(va, void *);
    }

    result = arena_alloc(arena, new_size, optional);
    if (result == NULL) {
        goto none;
    } else if (ptr == NULL) {
        goto none;
    }

    memmove(result, ptr, old_size > new_size ? new_size : old_size);
none:
    va_end(va);
    return result;
}

void arena_free(arena_t *arena)
{
    while (arena) {
        arena_t *tmp = arena->next;
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
