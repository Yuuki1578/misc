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

/*

Linked list, this is designed to be used in another data structure
(See the use case on arena allocator below).

The linked list only stored the @next pointer as its member,
the value is the offset from the base pointer + sizeof NodeLink.

API:
NodeLink* nodelink_init(usize size);
    Initialize a node with the size of @size.

NodeLink* nodelink_insert_after(NodeLink* node, usize size);
    Insert a new node after @node, preserve the newer node as
    its return value.

NodeLink* nodelink_insert_before(NodeLink* node, usize size);
    Insert a new node behind @node, return the node before @node.

usize nodelink_count_from(NodeLink* node);
    Count the length of a linked list from @node until @node is null.

void* nodelink_get_value(NodeLink* node);
    Get the inner value from @node on its offset in memory (just a math).

void nodelink_free_from(NodeLink* node);
    Free all node starting from @node.

*/

void* strict_alloc(usize size);
void* strict_realloc(void* ptr, usize size);

#ifdef MISC_IMPL
void* strict_alloc(usize size)
{
    void* p = calloc(size, 1);
    assert(p != NULL);
    return p;
}

void* strict_realloc(void* ptr, usize size)
{
    void* p = realloc(ptr, size);
    assert(p != NULL);
    return p;
}
#endif

typedef struct NodeLink NodeLink;
struct NodeLink {
    NodeLink* next;
    // ...
};

NodeLink* nodelink_init(usize size);
NodeLink* nodelink_insert_after(NodeLink* node, usize size);
NodeLink* nodelink_insert_before(NodeLink* node, usize size);
NodeLink* nodelink_remove_after(NodeLink* node);
NodeLink* nodelink_find_last(NodeLink* node);
usize nodelink_count_from(NodeLink* node);
void* nodelink_get_value(NodeLink* node);
void nodelink_free_from(NodeLink* node);

#ifdef MISC_IMPL
NodeLink* nodelink_init(usize size)
{
    NodeLink* node = malloc(sizeof *node + size);
    assert(node != NULL);
    node->next = NULL;
    return node;
}

NodeLink* nodelink_insert_after(NodeLink* node, usize size)
{
    NodeLink* next = nodelink_init(size);
    node->next = next;
    return next;
}

NodeLink* nodelink_insert_before(NodeLink* node, usize size)
{
    NodeLink* before = nodelink_init(size);
    assert(before != NULL);
    before->next = node;
    return before;
}

NodeLink* nodelink_remove_after(NodeLink* node)
{
    NodeLink* next = node->next;
    NodeLink* tmp = next != NULL ? next->next : NULL;
    node->next = tmp;
    return next;
}

NodeLink* nodelink_find_last(NodeLink* node)
{
    if (node == NULL) return NULL;
    while (node->next != NULL)
        node = node->next;

    return node;
}

void* nodelink_get_value(NodeLink* node)
{
    return (u8*)node + sizeof *node;
}

usize nodelink_count_from(NodeLink* node)
{
    usize count = 0;
    while (node != NULL)
        node = node->next,
        count++;

    return count;
}

void nodelink_free_from(NodeLink* node)
{
    while (node != NULL) {
        NodeLink* tmp = node->next;
        free(node);
        node = tmp;
    }
}
#endif

typedef struct Arena Arena;

Arena* arena_init(usize size);
void* arena_alloc(Arena* arena, usize size);
void* arena_realloc(Arena* arena, void* ptr, usize size_before, usize size_after);
void arena_free(Arena* arena);
usize arena_size(Arena* arena);

#ifdef MISC_IMPL
typedef struct {
    usize cap;
    usize len;
    // ...
} ArenaBody;

struct Arena {
    NodeLink
        *head,
        *last;
};

Arena* arena_init(usize size)
{
    assert(size > 0);
    Arena* arena = malloc(sizeof *arena);
    assert(arena != NULL);

    ArenaBody body = { .cap = size };
    arena->head = nodelink_init(sizeof body + size);
    arena->last = arena->head;

    ArenaBody* value = nodelink_get_value(arena->head);
    *value = body;
    return arena;
}

void* arena_alloc(Arena* arena, usize size)
{
    assert(arena != NULL && size > 0);

    NodeLink* last = arena->last;
    ArenaBody* body = nodelink_get_value(last);

    if (body->cap - body->len < size) {
        usize new_size = (body->cap > size ? body->cap : size) + size;
        ArenaBody newer = { .cap = new_size };
        NodeLink* new_tail = nodelink_insert_after(last, sizeof newer + new_size);

        arena->last = new_tail;
        last = arena->last;
        body = nodelink_get_value(last);
        *body = newer;
    }

    void* ptr = (u8*)body + sizeof *body + body->len;
    body->len += size;
    uintptr_t aligned = ((uintptr_t)ptr + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    return (void*)aligned;
}

void* arena_realloc(Arena* arena, void* ptr, usize size_before, usize size_after)
{
    if (size_after == 0) return NULL;

    void* newer = arena_alloc(arena, size_after);
    if (ptr == NULL) return newer;

    usize true_size = size_before > size_after ? size_after : size_before;
    return memmove(newer, ptr, true_size);
}

void arena_free(Arena* arena)
{
    if (arena != NULL) {
        nodelink_free_from(arena->head);
        free(arena);
    }
}

usize arena_size(Arena* arena)
{
    usize size = 0;
    if (arena == NULL) return size;

    NodeLink* node = arena->head;
    while (node != NULL) {
        ArenaBody* body = nodelink_get_value(node);
        size += body->cap;
        node = node->next;
    }
    return size;
}
#endif

#define MISC_ARRAY_RESERVE (8)

#define Array(T)    \
    struct {        \
        T* items;   \
        usize cap;  \
        usize len;  \
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
        assert(ok);                      \
    } while (0)

#define array_append(array, item)           \
    do {                                    \
        bool ok;                            \
        array_try_append(array, item, &ok); \
        assert(ok);                         \
    } while (0)

#define array_extend(array, many_ptr, N)           \
    do {                                           \
        bool ok;                                   \
        array_try_extend(array, many_ptr, N, &ok); \
        assert(ok);                                \
    } while (0)

#define array_remove_at(array, index)                                             \
    do {                                                                          \
        if ((array)->len > 1 && (index) < (array)->len) {                         \
            for (usize i = (index); i < (array)->len - 1; i++) {                    \
                (array)->items[i] = (array)->items[i + 1];                        \
            }                                                                     \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--;                                                       \
        }                                                                         \
    } while (0)

#define array_shrink_to_fit(array) array_resize(array, (array)->len)
#define array_free(array) array_resize(array, 0)

#define Slice(T)        \
    struct {            \
        const T* items; \
        usize len;      \
    }

typedef Array(char) String;
typedef Slice(char) StringRef;

// Exclusive
#define slice_from(slice, ptr, length, begin, end)    \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        usize _b, _e;                                   \
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

#ifdef MISC_IMPL
StringRef stringref_from(const char* cstr, usize begin, usize end)
{
    StringRef ref = {0};
    if (cstr == NULL || end < begin)
        return ref;

    usize len = strlen(cstr);
    slice_from(&ref, cstr, len, begin, end);
    return ref;
}

StringRef stringref_from_string(String* str, usize begin, usize end)
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
    String str = {0};
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
#define MISC_HASHMAP_INITCAP (16)
#endif

u64 fnv_init(const void* ptr, usize size);

#ifdef MISC_IMPL

u64 fnv_init(const void* ptr, usize size)
{
    const u8* bytes = ptr;
    u64 base_number = MISC_FNV_BASIS;
    for (u64 i = 0; i < size; i++) {
        base_number *= MISC_FNV_PRIME;
        base_number ^= bytes[i];
    }
    return base_number;
}

#endif

#endif
