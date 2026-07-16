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
Arena* arena_init(usize size, u32 flags, ...);
void* arena_alloc(Arena* arena, usize size, ...);
void* arena_realloc(Arena* arena, void* ptr, usize old_size, usize new_size, ...);
void arena_free(Arena* arena);

#define MISC_ARRAY_RESERVE (8)

#define Array(T)  \
    struct {      \
        T* items; \
        u32 cap;  \
        u32 len;  \
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
            for (u32 i = (index); i < (array)->len - 1; i++) {                    \
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
        u32 len;        \
    }

typedef Array(char) String;

typedef Slice(char) StringRef;

// Inclusive
#define slice_from(slice, ptr, length, begin, end)    \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        usize _b, _e;                                 \
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
StringRef stringref_from(const char* cstr, usize begin, usize end);
StringRef stringref_from_string(String* str, usize begin, usize end);
String string_printf(const char* fmt, ...);
char* cstring_printf(Arena* allocator, const char* fmt, ...);

#define MISC_FNV_BASIS (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME (0x100000001b3ULL)
#define MISC_FNV_LIMIT (64)
#ifndef MISC_HASHMAP_LOADFACTOR
#define MISC_HASHMAP_LOADFACTOR (0.75)
#endif
#ifndef MISC_HASHMAP_INITCAP
#define MISC_HASHMAP_INITCAP (8)
#endif

#define hashmap_get_index(hash, table_cap) ((hash) & ((table_cap) - 1))
#define MISC_HASHMAP_FOUND_NULL (0)
#define MISC_HASHMAP_FOUND_HEAD (1)
#define MISC_HASHMAP_FOUND_LIST (2)

typedef struct {
    u64 hash;
    void* key;
    usize len;
} HashKey;

typedef struct HashEntry {
    HashKey key;
    void* value;
    struct HashEntry* next;
} HashEntry;

typedef Array(HashEntry) RawHashTable;

typedef struct {
    RawHashTable table;
} HashMap;

#define hashmap_load_factor(map) ((double)(map)->table.len / (double)(map)->table.cap)
#define hashmap_put_cstr(map, cstr, value, size) hashmap_put(map, (HashKey) { .key = (void*)(cstr), .len = strlen(cstr) }, value, size)
#define hashmap_get_cstr(map, cstr) hashmap_get(map, (HashKey) { .key = (void*)(cstr), .len = strlen(cstr) })

u64 fnv_init(const void* ptr, const usize size);
void hashmap_put(HashMap* map, HashKey key, void* value, const usize size);
void* hashmap_get(const HashMap* map, const HashKey key);
bool hashmap_delete_at(HashMap* map, const HashKey key);
void hashmap_free(HashMap* map);

#ifdef MISC_IMPL
StringRef stringref_from(const char* cstr, usize begin, usize end)
{
    StringRef ref = { 0 };
    if (cstr == NULL || end < begin)
        return ref;

    usize len = strlen(cstr);
    slice_from(&ref, cstr, len, begin, end);
    return ref;
}

StringRef stringref_from_string(String* str, usize begin, usize end)
{
    StringRef ref = { 0 };
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
        if ((buf = arena_alloc(allocator, (usize)size + 1)) == NULL)
            goto end;

        va_start(va, fmt);
        vsnprintf(buf, (usize)size + 1, fmt, va);
        va_end(va);
    }

end:;
    return buf;
}

String string_printf(const char* fmt, ...)
{
    String str = { 0 };
    va_list va;
    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        bool ok;
        array_try_resize(&str, (usize)size + 1, &ok);
        if (!ok)
            return str;

        va_start(va, fmt);
        size = vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size > 0 ? size : 0;
    }

    return str;
}

u64 fnv_init(const void* ptr, const usize size)
{
    const u8* bytes = ptr;
    u64 base_number = MISC_FNV_BASIS;
    for (usize i = 0; i < size; i++) {
        base_number *= MISC_FNV_PRIME;
        base_number ^= bytes[i];
    }
    return base_number;
}

static bool hashentry_is_empty(const HashEntry* entry)
{
    /*
    We prevent the emptyness of an entry in hashentry_init().
    I really want to squeeze out performance as much as i can,
    so comparing it with memcmp() is a big no.
    */
    return entry->key.hash == 0;
}

static HashEntry* hashentry_find_tail(HashEntry* head)
{
    if (head == NULL)
        return NULL;

    HashEntry* current = head;
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
static bool hashmap_rehash(HashMap* map)
{
    RawHashTable* old_table = &map->table;
    RawHashTable new_table = { 0 };
    bool ok;
    array_try_resize(&new_table, old_table->cap * 2, &ok);
    if (!ok) return false;

    new_table.len = old_table->len;
    for (usize i = 0; i < old_table->cap; i++) {
        HashEntry* entry = &old_table->items[i];
        if (hashentry_is_empty(entry)) continue;

        // Literally life saver right here
        u64 new_index = hashmap_get_index(entry->key.hash, new_table.cap);
        HashEntry* new_entry = &new_table.items[new_index];

        if (hashentry_is_empty(new_entry)) {
            *new_entry = *entry;
        } else {
            HashEntry* tail = hashentry_find_tail(new_entry);
            tail->next = entry;
        }
    }

    array_free(old_table);
    map->table = new_table;
    return true;
}

static bool hashmap_try_reserve(HashMap* map)
{
    RawHashTable* table = &map->table;
    bool ok;

    if (table->cap < MISC_HASHMAP_INITCAP) {
        array_try_resize(table, MISC_HASHMAP_INITCAP, &ok);
        if (!ok) return false;
    }
    if (hashmap_load_factor(map) >= MISC_HASHMAP_LOADFACTOR) {
        return hashmap_rehash(map);
    }
    return true;
}

static bool hashentry_init(HashEntry* entry, HashKey hashed_key, void* value, usize size)
{
    entry->key.key = malloc(hashed_key.len);
    if (entry->key.key == NULL) return false;
    memmove(entry->key.key, hashed_key.key, hashed_key.len);

    entry->value = malloc(size);
    if (entry->value != NULL) {
        memmove(entry->value, value, size);
        entry->key.len = hashed_key.len;
        entry->key.hash = hashed_key.hash; // empty marker
        entry->next = NULL; // head marker
        return true;
    }

    free(entry->key.key), entry->key.key = NULL;
    return false;
}

static HashEntry* hashentry_find_exact(HashEntry* head, const HashKey key)
{
    u64 hash = fnv_init(key.key, key.len);
    while (head != NULL) {
        if (head->key.hash == hash && head->key.len == key.len) break;
        head = head->next;
    }
    return head;
}

static bool hashentry_is_head(HashEntry* maybe_head)
{
    return maybe_head->next == NULL;
}

void hashmap_put(HashMap* map, HashKey key, void* value, const usize size)
{
    if (key.key == NULL || value == NULL) return;
    if (!hashmap_try_reserve(map)) return;

    RawHashTable* table = &map->table;
    key.hash = fnv_init(key.key, key.len); // hash it first
    u64 index = hashmap_get_index(key.hash, table->cap);
    HashEntry* entry = &table->items[index];

    // Jackpot
    if (hashentry_is_empty(entry)) {
        if (hashentry_init(entry, key, value, size))
            table->len++;

    } else { // Not so lucky
        HashEntry* tail = hashentry_find_tail(entry);
        HashEntry* end = malloc(sizeof *end);
        if (end != NULL) {
            if (hashentry_init(end, key, value, size))
                tail->next = end;
        }
    }
}

/*
@params:
    @in      map
    @in/@out head
    @in      key
    @in/@out found_status
*/
static HashEntry* hashmap_get_entry(const HashMap* map, HashEntry** head, const HashKey key, int* found_status)
{
    const RawHashTable* table = &map->table;
    u64 hash = fnv_init(key.key, key.len);
    u64 index = hashmap_get_index(hash, table->cap);
    HashEntry* entry = &table->items[index];

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

void* hashmap_get(const HashMap* map, const HashKey key)
{
    int found;
    HashEntry* head;
    HashEntry* entry = hashmap_get_entry(map, &head, key, &found);
    if (entry != NULL)
        return entry->value;

    return NULL;
}

bool hashmap_delete_at(HashMap* map, const HashKey key)
{
    int found;
    HashEntry* head;
    HashEntry* entry = hashmap_get_entry(map, &head, key, &found);
    if (entry == NULL)
        return false;
    /* In case of the entry is a head of a list, do a memset 0 instead of free */
    if (found == MISC_HASHMAP_FOUND_HEAD) {
        free(entry->value);
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

    /*
    looping through head until head->next become entry, saving the entry parent
    node in head
    */
    HashEntry* entry_child = entry->next;
    free(entry->value);
    free(entry->key.key);
    while (head->next != entry)
        head = head->next;

    head->next = entry_child;
    free(entry);
    return true;
}

void hashmap_free(HashMap* map)
{
    for (usize i = 0; i < map->table.cap; i++) {
        HashEntry* entry = &map->table.items[i];
        if (hashentry_is_empty(entry)) continue;

        HashEntry *node = entry, *next = NULL;
        while (node != NULL) {
            free(node->value);
            free(node->key.key);
            next = node->next;

            /* if node == entry, which is the head, skip free
             * because it's managed by array_* API
             * */
            if (node != entry) free(node); /* Not head */
            node = next;
        }
    }
    array_free(&map->table);
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

Arena* arena_init(usize size, u32 flags, ...)
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

static Arena* arena_find_exact(Arena* arena, usize size, int* found)
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

void* arena_alloc(Arena* arena, usize size, ...)
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

void* arena_realloc(Arena* arena, void* ptr, usize old_size, usize new_size, ...)
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

#endif
