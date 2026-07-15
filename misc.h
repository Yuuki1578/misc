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

#define MISC_VOIDPTR(expr) ((void*)(expr))
#define MISC_ARENA_PAGESIZE (1ULL << 12ULL)
#define MISC_ARSTACK (0x1)
#define MISC_ARHEAP (0x10)
#define MISC_ARNOGROW (0x100)
#define MISC_ARDEFAULT (MISC_ARHEAP)

typedef struct arena arena_t;
struct arena {
    arena_t* next;
    uint32_t total, offset, flags;
};

#define arena_remains(a) ((a)->total - (a)->offset)
arena_t* arena_init(size_t size, uint32_t flags, ...);
void* arena_alloc(arena_t* arena, size_t size, ...);
void* arena_realloc(arena_t* arena, void* ptr, size_t old_size, size_t new_size, ...);
void arena_free(arena_t* arena);

#define MISC_ARRAY_RESERVE (8)

#define array_t(type) \
    struct {          \
        type* items;  \
        uint32_t cap; \
        uint32_t len; \
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

#define array_try_extend(array, many_ptr, N, ok)                                      \
    do {                                                                              \
        if ((many_ptr) != NULL && (N) > 0) {                                          \
            if (array_is_empty(array) || array_remains(array) <= (N)) {               \
                array_try_resize(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok); \
                if (!*(ok)) {                                                         \
                    break;                                                            \
                }                                                                     \
            }                                                                         \
            memmove((array)->items + (array)->len, (many_ptr),                        \
                (N) * sizeof *(array)->items);                                        \
            (array)->len += (N);                                                      \
            *(ok) = 1;                                                                \
        } else {                                                                      \
            *(ok) = 0;                                                                \
        }                                                                             \
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
            for (uint32_t i = (index); i < (array)->len - 1; i++) {               \
                (array)->items[i] = (array)->items[i + 1];                        \
            }                                                                     \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--;                                                       \
        }                                                                         \
    } while (0)

#define array_free(array) array_resize(array, 0)

#define slice_t(type)      \
    struct {               \
        const type* items; \
        uint32_t len;      \
    }

typedef array_t(char) str_t;

typedef slice_t(char) stref_t;

// Inclusive
#define slice_from(slice, ptr, length, begin, end)    \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        size_t _b, _e;                                \
        _b = (begin) > (length) ? (length) : (begin); \
        _e = (end) > (length) ? (length) : (end);     \
        (slice)->items = (ptr) + (_b);                \
        (slice)->len = ((_e) - (_b));                 \
    } while (0)

#define slice_from_array(slice, array, begin, end) slice_from(slice, (array)->items, (array)->len, begin, end)

/*
Legends:
    function with prefixes str_* is going to use traditional char*
    function with prefixes string_* is going to use str_t
    function with prefixes stref_* is going to use stref_t
*/

#define str_fmt(str) (int)(str).len, (str).items
stref_t stref_from(const char* cstr, size_t begin, size_t end);
stref_t stref_from_str(str_t* str, size_t begin, size_t end);
str_t string_printf(const char* fmt, ...);
char* cstr_printf(arena_t* allocator, const char* fmt, ...);

#define MISC_FNV_BASIS (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME (0x100000001b3ULL)
#define MISC_FNV_LIMIT (64)
#define MISC_HASHMAP_LOADFACTOR (0.75)
#ifndef MISC_HASHMAP_INITCAP
#define MISC_HASHMAP_INITCAP (8)
#endif

#define MISC_HASHMAP_GETINDEX(hash, table_cap) ((hash) % (table_cap))
#define MISC_HASHMAP_FOUND_NULL (0)
#define MISC_HASHMAP_FOUND_HEAD (1)
#define MISC_HASHMAP_FOUND_LIST (2)

typedef struct {
    uint64_t hash;
    uint8_t* key;
    size_t len;
} hashkey_t;

typedef struct hashentry {
    hashkey_t key;
    void* value;
    struct hashentry* next;
} hashentry_t;

typedef array_t(hashentry_t) raw_table_t;

typedef struct {
    raw_table_t table;
} hashmap_t;

#define hashmap_loadfactor(map) ((double)(map)->table.len / (double)(map)->table.cap)
#define hashmap_put_cstr(map, cstr, value, size) hashmap_put(map, (hashkey_t) { .key = (void*)(cstr), .len = strlen(cstr) }, value, size)
#define hashmap_get_cstr(map, cstr) hashmap_get(map, (hashkey_t) { .key = (void*)(cstr), .len = strlen(cstr) })

uint64_t fnv_init(const void* ptr, const size_t size);
void hashmap_put(hashmap_t* map, hashkey_t key, void* value, const size_t size);
void* hashmap_get(const hashmap_t* map, const hashkey_t key);
bool hashmap_delete_at(hashmap_t* map, const hashkey_t key);
void hashmap_free(hashmap_t* map);

#ifdef MISC_IMPL
stref_t stref_from(const char* cstr, size_t begin, size_t end)
{
    stref_t ref = { 0 };
    if (cstr == NULL || end < begin)
        return ref;

    size_t len = strlen(cstr);
    slice_from(&ref, cstr, len, begin, end);
    return ref;
}

stref_t stref_from_string(str_t* str, size_t begin, size_t end)
{
    stref_t ref = { 0 };
    slice_from_array(&ref, str, begin, end);
    return ref;
}

char* cstr_printf(arena_t* allocator, const char* fmt, ...)
{
    char* buf = NULL;
    va_list va;

    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        if ((buf = arena_alloc(allocator, (size_t)size + 1)) == NULL)
            goto end;

        va_start(va, fmt);
        vsnprintf(buf, (size_t)size + 1, fmt, va);
        va_end(va);
    }

end:;
    return buf;
}

str_t str_printf(const char* fmt, ...)
{
    str_t str = { 0 };
    va_list va;
    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        bool ok;
        array_try_resize(&str, (size_t)size + 1, &ok);
        if (!ok)
            return str;

        va_start(va, fmt);
        size = vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size > 0 ? size : 0;
    }

    return str;
}

uint64_t fnv_init(const void* ptr, const size_t size)
{
    const uint8_t* bytes = ptr;
    uint64_t base_number = MISC_FNV_BASIS;
    for (size_t i = 0; i < size; i++) {
        base_number *= MISC_FNV_PRIME;
        base_number ^= bytes[i];
    }
    return base_number;
}

static bool hashentry_is_empty(const hashentry_t* entry)
{
    hashentry_t zeroed = { 0 };
    return memcmp(entry, &zeroed, sizeof zeroed) == 0;
}

static hashentry_t* hashentry_find_tail(hashentry_t* head)
{
    if (head == NULL)
        return NULL;

    hashentry_t* current = head;
    while (current->next != NULL)
        current = current->next;

    return current;
}

/*
IMPORTANT:
    From previous commit, i use hashmap_rehash to rehash the table from the new array
    instead of the old one, and this is the fix, basically add one more params (new_table),
    rehash based on new_table capacity, free the old table from map, replace the
    map->table with the new_table, and that's it.

    Duh, i spent a lot of time figuring out why this shit won't work oh my god, sorry, i don't
    go into college so this technical detail about data structure is very confusing.
*/
static void hashmap_rehash(hashmap_t* map, raw_table_t new_table)
{
    raw_table_t* old_table = &map->table;

    for (size_t i = 0; i < old_table->cap; i++) {
        hashentry_t* entry = &old_table->items[i];
        if (hashentry_is_empty(entry))
            continue;

        // Literally life saver right here
        uint64_t new_index = MISC_HASHMAP_GETINDEX(entry->key.hash, new_table.cap);
        hashentry_t* new_entry = &new_table.items[new_index];

        if (hashentry_is_empty(new_entry)) {
            *new_entry = *entry;
        } else {
            hashentry_t* tail = hashentry_find_tail(new_entry);
            tail->next = entry;
        }
    }

    array_free(old_table);
    map->table = new_table;
}

static bool hashmap_try_reserve(hashmap_t* map)
{
    raw_table_t* table = &map->table;
    bool ok;

    if (table->cap < MISC_HASHMAP_INITCAP) {
        array_try_resize(table, MISC_HASHMAP_INITCAP, &ok);
        if (!ok)
            return false;
    }
    if (hashmap_loadfactor(map) >= MISC_HASHMAP_LOADFACTOR) {
        raw_table_t new_table = { 0 };
        bool ok;
        array_try_resize(&new_table, table->cap * 2, &ok);
        if (!ok)
            return false;

        new_table.len = table->len;
        hashmap_rehash(map, new_table);
    }
    return true;
}

static hashentry_t* hashentry_init(hashkey_t key, void* value)
{
    hashentry_t* entry = malloc(sizeof *entry);
    if (entry == NULL)
        return NULL;

    memset(entry, 0, sizeof *entry);
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static hashentry_t* hashentry_find_exact(hashentry_t* head, const hashkey_t key)
{
    uint64_t hash = fnv_init(key.key, key.len);
    while (head != NULL) {
        if (head->key.hash == hash && head->key.len == key.len)
            break;

        head = head->next;
    }
    return head;
}

static bool hashentry_is_head(hashentry_t* maybe_head)
{
    return maybe_head->next == NULL;
}

void hashmap_put(hashmap_t* map, hashkey_t key, void* value, const size_t size)
{
    if (!hashmap_try_reserve(map))
        return;

    raw_table_t* table = &map->table;
    key.hash = fnv_init(key.key, key.len);
    uint64_t index = MISC_HASHMAP_GETINDEX(key.hash, table->cap);
    hashentry_t* entry = &table->items[index];

    bool alloc_key = false, alloc_value = false;
    hashentry_t appended = {
        .key = key,
        .value = value,
        .next = NULL,
    };

    if (key.key != NULL && key.len > 0) {
        appended.key.key = malloc(key.len);
        if (appended.key.key == NULL)
            return;

        memmove(appended.key.key, key.key, key.len), alloc_key = true;
    }
    if (value != NULL && size > 0) {
        appended.value = malloc(size);
        if (appended.value == NULL)
            return;

        memmove(appended.value, value, size), alloc_value = true;
    }

    // Jackpot
    if (hashentry_is_empty(entry)) {
        *entry = appended;
        table->len++;

        // Not so lucky
    } else {
        hashentry_t* tail = hashentry_find_tail(entry);
        if (tail == NULL)
            return;

        hashentry_t* end = hashentry_init(appended.key, appended.value);
        if (end == NULL) {
            if (alloc_key)
                free(appended.key.key);
            if (alloc_value)
                free(appended.value);

            return;
        }

        tail->next = end;
    }
}

static hashentry_t* hashmap_get_entry(const hashmap_t* map, hashentry_t** head, const hashkey_t key, int* found_status)
{
    const raw_table_t* table = &map->table;
    uint64_t hash = fnv_init(key.key, key.len);
    uint64_t index = MISC_HASHMAP_GETINDEX(hash, table->cap);
    hashentry_t* entry = &table->items[index];

    if (hashentry_is_empty(entry)) {
        *head = NULL;
        *found_status = MISC_HASHMAP_FOUND_NULL;
        return NULL;
    } else if (hashentry_is_head(entry)) {
        *head = entry;
        *found_status = MISC_HASHMAP_FOUND_HEAD;
        return entry; /* Jackpot */
    }
    *head = entry;
    *found_status = MISC_HASHMAP_FOUND_LIST;
    return hashentry_find_exact(*head, key);
}

void* hashmap_get(const hashmap_t* map, const hashkey_t key)
{
    int found;
    hashentry_t* head;
    hashentry_t* entry = hashmap_get_entry(map, &head, key, &found);
    if (entry != NULL)
        return entry->value;

    return NULL;
}

bool hashmap_delete_at(hashmap_t* map, const hashkey_t key)
{
    int found;
    hashentry_t* head;
    hashentry_t* entry = hashmap_get_entry(map, &head, key, &found);
    if (entry == NULL)
        return false;
    /* In case of the entry is a head of a list, do a memset 0 instead of free */
    if (found == MISC_HASHMAP_FOUND_HEAD) {
        if (entry->value != NULL)
            free(entry->value);
        if (entry->key.key != NULL && entry->key.len > 0)
            free(entry->key.key);

        /*
        TODO: Calculate the pointer diff (entry - map->table.items) if entry is the head
        of a list to use that as an index to an entry target in an array, remove that from
        the array, reduce the capacity of the array, and rehash the entire table.
        PRO: Minimize memory usage
        CONS: Maybe slower because i need to rehash on 2 different context:
            1. When the table's capacity increase
            2. When i need to remove an entry from the array (if only it's a head of a list)
        */
        memset(entry, 0, sizeof *entry);
        return true;
    }

    /* looping through head until head->next become entry, saving the entry parent
     * node in head */
    hashentry_t* entry_child = entry->next;
    if (entry->value != NULL)
        free(entry->value);
    if (entry->key.key != NULL && entry->key.len > 0)
        free(entry->key.key);
    while (head->next != entry)
        head = head->next;

    head->next = entry_child;
    free(entry);
    return true;
}

void hashmap_free(hashmap_t* map)
{
    for (size_t i = 0; i < map->table.cap; i++) {
        hashentry_t* entry = &map->table.items[i];
        if (hashentry_is_empty(entry))
            continue;

        hashentry_t *node = entry, *next = NULL;
        while (node != NULL) {
            if (node->value != NULL)
                free(node->value);
            if (node->key.key != NULL && node->key.len > 0)
                free(node->key.key);

            next = node->next;

            /* if node == entry, which is the head, skip free
             * because it's managed by array_* API
             * */
            if (node != entry)
                free(node); /* Not head */
            node = next;
        }
    }
    array_free(&map->table);
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

arena_t* arena_init(size_t size, uint32_t flags, ...)
{
    arena_t* head_node = NULL;
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

    if (!head_node)
        goto none;

    head_node->next = NULL;
    head_node->total = flags & MISC_ARSTACK ? size - sizeof *head_node : size;
    head_node->offset = 0;
    head_node->flags = flags;

none:
    va_end(va);
    return head_node;
}

static arena_t* arena_find_exact(arena_t* arena, size_t size, int* found)
{
    arena_t *visitor = arena, *last_nonnull = NULL;
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

void* arena_alloc(arena_t* arena, size_t size, ...)
{
    arena_t* suitable;
    va_list va;
    int found = 0;
    void* result = NULL;

    if (arena == NULL || size == 0)
        return NULL;

    va_start(va, size);
    if (size > arena_remains(arena)) {
        suitable = arena_find_exact(arena, size, &found);
    } else {
        suitable = arena, found = 1;
    }

    if (!found) {
        void* optional = NULL;
        if (suitable->flags & MISC_ARNOGROW)
            goto none;

        if (suitable->flags & MISC_ARSTACK)
            optional = va_arg(va, void*);

        suitable->next = arena_init(size + suitable->total, suitable->flags, optional);
        if (!suitable->next)
            goto none;

        suitable = suitable->next;
    }

    uint8_t* offset_ptr = (uint8_t*)MISC_VOIDPTR(suitable) + sizeof *suitable;
    result = offset_ptr + suitable->offset;
    suitable->offset = suitable->offset + size;

none:
    va_end(va);
    return result;
}

void* arena_realloc(arena_t* arena, void* ptr, size_t old_size, size_t new_size, ...)
{
    void* optional = NULL;
    void* result = NULL;
    va_list va;

    if (!arena)
        goto none;

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

void arena_free(arena_t* arena)
{
    while (arena) {
        arena_t* tmp = arena->next;
        if (arena->flags & MISC_ARHEAP)
            free(arena);
        else
            memset(arena, 0, arena->total + sizeof *arena);

        arena = tmp;
    }
}

#endif

#endif
