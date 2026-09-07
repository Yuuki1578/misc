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

#include <ctype.h>
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
typedef uintptr_t usize;
typedef intptr_t isize;
typedef float f32;
typedef double f64;
typedef long double f80;

#define MISC_ALIGN (sizeof(void*))
#define fprintfn(f, fmt, ...) fprintf(f, fmt "\n", __VA_ARGS__)
#define printfn(fmt, ...) fprintfn(stdout, fmt, __VA_ARGS__)

/*

Common interface to allocate memory in a uniform manner.
@any could be anything, yes, ANYTHING.
@size is a size, of course.
@alignment must be a power of 2, if not, the creator must explicitly return NULL.
@alignment is then packed into @size and call allocation API, so the allocated
memory is aligned. The alignment is applied before and not after it's allocated.

This header exposes 2 kind of allocators:
1. Libc allocator as @libc_alloc.
2. System mapped virtual memory as @mmap_alloc.

For most of the time, you should use the @libc_alloc, as it will uses
the malloc(3), realloc(3) and free(3) with the addition of an alignment in it.

the implemented allocators here will store additional address metadata such as its size
in allocated memory, on windows it also stores the HANDLE (basically a fat pointer).
Maybe we could make it store the alignment, so we can get more clean function table here haha.

NOTE:
allocator -> alloc :
    * Returned memory must be ZEROED
    * Alignment are handled before the allocation
*/
struct allocator {
    void *any;
    void *(*alloc)(void *any, usize size, usize alignment);
    void *(*realloc)(void *any, void *ptr, usize size, usize alignment);
    void (*free)(void *any, void *ptr);
};

extern struct allocator *const libc_alloc;
extern struct allocator *const mmap_alloc;

#define misc_palign(ptr, align) ((void*)(((uintptr_t)(ptr) + (align) - 1) & ~((align) - 1)))
#define misc_align_up(size) ((uintptr_t)misc_palign(size, 8))

void *misc_mmap(usize size, usize alignment);
void *misc_remap(void *ptr, usize size, usize alignment);
void misc_unmap(void *ptr);

#ifdef __unix__
#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES > 0
#define MISC_POSIX_MAP
#define MISC_EMAP MAP_FAILED
#else
#define MISC_EMAP NULL
#endif

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define MISC_EMAP NULL
#define MISC_WINAPI

#else
#define MISC_EMAP NULL
#endif

#define misc_alloc(size, alignment) calloc((usize)misc_palign(size, alignment), 1)
#define misc_realloc(ptr, size, alignment) realloc(ptr, (uintptr_t)misc_palign(size, alignment))
#define misc_free(ptr) free(ptr)

#ifdef MISC_IMPL

void *_misc_alloc(void *any, usize size, usize alignment)
{
    (void)any;
    return misc_alloc(size, alignment);
}

void *_misc_realloc(void *any, void *ptr, usize size, usize alignment)
{
    (void)any;
    return misc_realloc(ptr, size, alignment);
}

void _misc_free(void *any, void *ptr)
{
    (void)any;
    misc_free(ptr);
}

static struct allocator _libc_alloc = {
    .alloc = _misc_alloc,
    .realloc = _misc_realloc,
    .free = _misc_free,
};

struct allocator *const libc_alloc = &_libc_alloc;

void *misc_mmap(usize size, usize alignment)
{
    void *ptr;

    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
        return NULL;

#ifdef MISC_POSIX_MAP
    size = (usize)(uintptr_t)misc_palign(size + sizeof size, alignment);
    ptr = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);

#elif defined(MISC_WINAPI)
    size = (uintptr_t)misc_palign(size + sizeof size, alignment);
    ptr = VirtualAlloc(
        NULL,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

#else
    ptr = misc_alloc(size, alignment);
#endif

    if (ptr == MISC_EMAP)
        return NULL;

#if defined(MISC_POSIX_MAP) || defined(MISC_WINAPI)
    *(usize*)ptr = size;
    ptr = (u8*)ptr + sizeof size;
#endif

    return ptr;
}

void *misc_remap(void *ptr, usize size, usize alignment)
{
    void *newer;

    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
        return NULL;

    if (ptr == NULL)
        return misc_mmap(size, alignment);

#if defined(MISC_POSIX_MAP) || defined(MISC_WINAPI)
    usize *true_size = (void*)((u8*)ptr - sizeof(usize));
    usize copied = *true_size > size ? size : *true_size;
    newer = misc_mmap(size, alignment);

    if (newer == NULL)
        return NULL;

    memmove(newer, ptr, copied);
    misc_unmap(ptr);

#else
    newer = misc_realloc(ptr, size, alignment);
#endif

    return newer;
}

void misc_unmap(void *ptr)
{
    usize true_size;

    if (ptr == NULL)
        return;

#if defined(MISC_POSIX_MAP) || defined(MISC_WINAPI)
    ptr = (u8*)ptr - sizeof(usize);
    true_size = *(usize*)ptr;

#ifdef MISC_POSIX_MAP
    munmap(ptr, true_size);
#else
    (void)true_size;
    VirtualFree(ptr, 0, MEM_RELEASE);
#endif

#else
    misc_free(ptr);
#endif
}

void *_misc_mmap(void *any, usize size, usize alignment)
{
    (void)any;
    return misc_mmap(size, alignment);
}

void *_misc_remap(void *any, void *ptr, usize size, usize alignment)
{
    (void)any;
    return misc_remap(ptr, size, alignment);
}

void _misc_unmap(void *any, void *ptr)
{
    (void)any;
    misc_unmap(ptr);
}

static struct allocator _mmap_alloc = {
    .alloc = _misc_mmap,
    .realloc = _misc_remap,
    .free = _misc_unmap,
};

struct allocator *const mmap_alloc = &_mmap_alloc;

#endif

#define misc_panic(msg)                                                                   \
    do {                                                                                  \
        fprintfn(stderr, "FILE: %s, LINE: %d, cause: \"%s\"", __FILE__, __LINE__, (msg)); \
        abort();                                                                          \
    } while (0)

#define misc_assert(cond, msg) \
    do {                       \
        if (!(cond))           \
            misc_panic(msg);   \
    } while (0)

struct linked_list {
    struct linked_list *next;
    // ...
};

struct linked_list *ll_init_with(struct allocator *alloc, usize size);
struct linked_list *ll_put_after_with(struct allocator *alloc, struct linked_list *node, usize size);
struct linked_list *ll_put_before_with(struct allocator *alloc, struct linked_list *node, usize size);
void ll_free_with(struct allocator *alloc, struct linked_list *node);

#define ll_init(size) ll_init_with(libc_alloc, size)
#define ll_put_after(node, size) ll_put_after_with(libc_alloc, node, size)
#define ll_put_before(node, size) ll_put_before_with(libc_alloc, node, size)
#define ll_free(node) ll_free_with(libc_alloc, node)

struct linked_list *ll_pop_after(struct linked_list *node);
struct linked_list *ll_last(struct linked_list *node);
usize ll_len(struct linked_list *node);
void *ll_value(struct linked_list *node);

#ifdef MISC_IMPL
struct linked_list *ll_init_with(struct allocator *alloc, usize size)
{
    struct linked_list *node = alloc->alloc(alloc->any, size, 1);
    if (node != NULL)
        node->next = NULL;

    return node;
}

struct linked_list *ll_put_after_with(struct allocator *alloc, struct linked_list *node, usize size)
{
    struct linked_list *next = ll_init_with(alloc, size);
    node->next = next;
    return next;
}

struct linked_list *ll_put_before_with(struct allocator *alloc, struct linked_list *node, usize size)
{
    struct linked_list *before = ll_init_with(alloc, size);
    if (before != NULL)
        before->next = node;

    return before;
}

void ll_free_with(struct allocator *alloc, struct linked_list *node)
{
    while (node != NULL) {
        struct linked_list* tmp = node->next;
        alloc->free(alloc->any, node);
        node = tmp;
    }
}

struct linked_list *ll_pop_after(struct linked_list *node)
{
    struct linked_list *next = node->next;
    struct linked_list *tmp = next != NULL ? next->next : NULL;
    node->next = tmp;
    return next;
}

struct linked_list *ll_last(struct linked_list *node)
{
    if (node == NULL)
        return NULL;
    while (node->next != NULL)
        node = node->next;

    return node;
}

void *ll_value(struct linked_list *node)
{
    return (u8*)node + sizeof *node;
}

usize ll_len(struct linked_list *node)
{
    usize count = 0;
    while (node != NULL)
        node = node->next,
        count++;

    return count;
}
#endif

typedef struct arena arena_t;

arena_t *arena_init_with(struct allocator *alloc, usize size);
void *arena_alloc_with(struct allocator *alloc, arena_t *arena, usize size);
void *arena_realloc_with(struct allocator *alloc, arena_t *arena, void *ptr, usize size_before, usize size_after);
void arena_free_with(struct allocator *alloc, arena_t *arena);
bool arena_align_with(struct allocator *alloc, arena_t *arena, usize alignment);

#define arena_init(size) arena_init_with(libc_alloc, size)
#define arena_alloc(arena, size) arena_alloc_with(libc_alloc, arena, size)
#define arena_realloc(arena, ptr, size_before, size_after) arena_realloc_with(libc_alloc, arena, ptr, size_before, size_after)
#define arena_free(arena) arena_free_with(libc_alloc, arena)
#define arena_align(arena, alignment) arena_align_with(libc_alloc, arena, alignment)

usize arena_size(arena_t *arena);

#ifdef MISC_IMPL
struct arena_body {
    usize cap;
    usize len;
    // ...
};

struct arena {
    struct linked_list *head, *last;
};

arena_t *arena_init_with(struct allocator *alloc, usize size)
{
    arena_t *arena;
    struct arena_body body = {0}, *value;

    if (size < 1)
        return NULL;

    arena = alloc->alloc(alloc->any, sizeof *arena, 1);
    if (arena == NULL)
        return NULL;

    body.cap = size;
    arena->head = ll_init_with(alloc, sizeof body + size);
    if (arena->head == NULL) {
        alloc->free(alloc->any, arena);
        return NULL;
    }

    arena->last = arena->head;
    value = ll_value(arena->head);
    *value = body;
    return arena;
}

void *arena_alloc_with(
    struct allocator *alloc,
    arena_t *arena,
    usize size)
{
    struct linked_list *last;
    struct arena_body *body;
    void *ptr;

    if (arena == NULL || size < 1)
        return NULL;

    last = arena->last;
    body = ll_value(last);

    if (body->cap - body->len < size) {
        usize new_size = (body->cap > size ? body->cap : size) + size;
        struct arena_body newer = { .cap = new_size };
        struct linked_list *new_tail = ll_put_after_with(alloc, last, sizeof newer + new_size);

        if (new_tail == NULL)
            return NULL;

        arena->last = new_tail;
        last = arena->last;
        body = ll_value(last);
        *body = newer;
    }

    ptr = (u8*)body + sizeof *body + body->len;
    body->len += size;
    return ptr;
}

void *arena_realloc_with(
    struct allocator *alloc,
    arena_t *arena,
    void *ptr,
    usize size_before,
    usize size_after)
{
    void *newer;
    usize true_size;

    if (arena == NULL || size_after == 0)
        return NULL;

    newer = arena_alloc_with(alloc, arena, size_after);
    if (ptr == NULL)
        return newer;

    true_size = size_before > size_after ? size_after : size_before;
    return memmove(newer, ptr, true_size);
}

void arena_free_with(struct allocator *alloc, arena_t *arena)
{
    if (arena != NULL) {
        ll_free_with(alloc, arena->head);
        alloc->free(alloc->any, arena);
    }
}

usize arena_size(arena_t *arena)
{
    struct linked_list *node;
    usize size = 0;

    if (arena == NULL)
        return size;

    node = arena->head;
    while (node != NULL) {
        struct arena_body *body = ll_value(node);
        size += body->cap;
        node = node->next;
    }
    return size;
}

bool arena_align_with(struct allocator *alloc, arena_t *arena, usize alignment)
{
    struct arena_body *body, newer;
    struct linked_list *next;
    usize aligned;

    body = ll_value(arena->last);
    aligned = (usize)misc_palign(body->len, alignment);

    if (aligned >= body->cap) {
        newer = (struct arena_body) {.cap = aligned};
        if ((next = ll_put_after_with(alloc, arena->last, sizeof newer)) != NULL) {
            *(struct arena_body*) ll_value(next) = newer;
        } else return false;
    } else {
        body->len = aligned;
    }

    return true;
}

#endif

struct memory_pool {
    void *buffer;
    usize cap, len;
};

void *mp_alloc(struct memory_pool *pool, usize size);
void mp_clear(struct memory_pool *pool);

#ifdef MISC_IMPL
void *mp_alloc(struct memory_pool *pool, usize size)
{
    void *buffer;

    if (size == 0 || size > pool->cap - pool->len)
        return NULL;

    buffer = (u8*)pool->buffer + pool->len;
    pool->len += size;
    return buffer;
}

void mp_clear(struct memory_pool *pool)
{
    memset(pool->buffer, 0, pool->cap);
    pool->len = 0;
}

#endif

#ifndef MISC_ARRAY_RESERVE
#define MISC_ARRAY_RESERVE (8)
#endif

#define array_is_empty(array) ((array) != NULL ? ((array)->items == NULL || (array)->cap < 1) : 1)
#define array_remains(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define array_try_resize_with(allocator, array, N, ok)                                                         \
    do {                                                                                                       \
        if ((N) <= 0) {                                                                                        \
            (allocator)->free((allocator)->any, (array)->items);                                               \
            (array)->items = NULL;                                                                             \
            (array)->cap = 0;                                                                                  \
            (array)->len = 0;                                                                                  \
            *(ok) = 1;                                                                                         \
        } else {                                                                                               \
            void *tmp;                                                                                         \
            if ((array)->cap == 0) {                                                                           \
                tmp = (allocator)->alloc((allocator)->any, (N) * sizeof *(array)->items, 1);                   \
            } else {                                                                                           \
                tmp = (allocator)->realloc((allocator)->any, (array)->items, (N) * sizeof *(array)->items, 1); \
            }                                                                                                  \
            if (tmp != NULL) {                                                                                 \
                *(ok) = 1;                                                                                     \
                (array)->items = tmp;                                                                          \
                (array)->cap = (N);                                                                            \
                if ((N) < (array)->len) {                                                                      \
                    (array)->len = (N);                                                                        \
                }                                                                                              \
            } else {                                                                                           \
                *(ok) = 0;                                                                                     \
            }                                                                                                  \
        }                                                                                                      \
    } while (0)

#define array_try_resize(array, N, ok) array_try_resize_with(libc_alloc, array, N, ok)
#define array_resize(array, N) array_resize_with(libc_alloc, array, N)
#define array_resize_with(allocator, array, N)             \
    do {                                                   \
        bool ok;                                           \
        array_try_resize_with(allocator, array, N, &ok);   \
        misc_assert(ok, "array_try_resize_with() failed"); \
    } while (0)

#define array_try_append_with(allocator, array, item, ok)                                   \
    do {                                                                                    \
        if ((array)->cap <= (array)->len) {                                                 \
            array_try_resize_with(allocator, array, (array)->cap + MISC_ARRAY_RESERVE, ok); \
        }                                                                                   \
        if (*(ok)) {                                                                        \
            (array)->items[(array)->len++] = (item);                                        \
        }                                                                                   \
    } while (0)

#define array_append(array, item) array_append_with(libc_alloc, array, item)
#define array_try_append(array, item, ok) array_try_append_with(libc_alloc, array, item, ok)
#define array_append_with(allocator, array, item)           \
    do {                                                    \
        bool ok;                                            \
        array_try_append_with(allocator, array, item, &ok); \
        misc_assert(ok, "array_try_append_with() failed");  \
    } while (0)

#define array_try_extend_with(allocator, array, n_items, N, ok)                                       \
    do {                                                                                              \
        if ((n_items) != NULL && (N) > 0) {                                                           \
            if (array_is_empty(array) || array_remains(array) <= (N)) {                               \
                array_try_resize_with(allocator, array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok); \
                if (!*(ok)) {                                                                         \
                    break;                                                                            \
                }                                                                                     \
            }                                                                                         \
            memmove((array)->items + (array)->len, (n_items), (N) * sizeof *(array)->items);          \
            (array)->len += (N);                                                                      \
            *(ok) = 1;                                                                                \
        } else {                                                                                      \
            *(ok) = 0;                                                                                \
        }                                                                                             \
    } while (0)

#define array_extend(array, n_items, N) array_extend_with(libc_alloc, array, n_items, N)
#define array_try_extend(array, n_items, N, ok) array_try_extend_with(libc_alloc, array, n_items, N, ok)
#define array_extend_with(allocator, array, n_items, N)           \
    do {                                                          \
        bool ok;                                                  \
        array_try_extend_with(allocator, array, n_items, N, &ok); \
        misc_assert(ok, "array_try_extend_with() failed");        \
    } while (0)

#define array_remove_at(array, index)                                             \
    do {                                                                          \
        if ((array)->len > 1 && (index) < (array)->len) {                         \
            for (usize i = (index); i < (array)->len - 1; i++) {                  \
                (array)->items[i] = (array)->items[i + 1];                        \
            }                                                                     \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--;                                                       \
        }                                                                         \
    } while (0)

#define array_reverse(T, array)                               \
    do {                                                      \
        if ((array)->len > 1) {                               \
            usize front = 0, back = (array)->len - 1;         \
            while (front < back) {                            \
                T tmp = (array)->items[front];                \
                (array)->items[front] = (array)->items[back]; \
                (array)->items[back] = tmp;                   \
                front++, back--;                              \
            }                                                 \
        }                                                     \
    } while (0)

#define array_try_append_at_with(allocator, array, idx, item, ok)                                                           \
    do {                                                                                                                    \
        if ((idx) < (array)->len) {                                                                                         \
            if ((array)->cap - (array)->len <= 1) {                                                                         \
                array_try_resize_with(allocator, array, (array)->cap + MISC_ARRAY_RESERVE, ok);                             \
                if (!*(ok))                                                                                                 \
                    break;                                                                                                  \
            }                                                                                                               \
            memmove((array)->items + ((idx) + 1), (array)->items + (idx), ((array)->len - (idx)) * sizeof *(array)->items); \
            (array)->items[(idx)] = (item);                                                                                 \
            (array)->len++;                                                                                                 \
            *(ok) = 1;                                                                                                      \
        } else {                                                                                                            \
            array_try_append_with(allocator, array, item, ok);                                                              \
        }                                                                                                                   \
    } while (0)

#define array_try_append_at(array, idx, item, ok) array_try_append_at_with(libc_alloc, array, idx, item, ok)
#define array_append_at(array, idx, item) array_append_at_with(libc_alloc, array, idx, item)
#define array_append_at_with(allocator, array, idx, item)           \
    do {                                                            \
        bool ok;                                                    \
        array_try_append_at_with(allocator, array, idx, item, &ok); \
        misc_assert(ok, "array_try_append_with() failed");          \
    } while (0)

#define array_make_fit(array) array_make_fit_with(libc_alloc, array)
#define array_make_fit_with(allocator, array)                       \
    do {                                                            \
        bool ok;                                                    \
        array_try_resize_with(allocator, array, (array)->len, &ok); \
        (void)ok;                                                   \
    } while (0)

#define array_free(array) array_free_with(libc_alloc, array)
#define array_free_with(allocator, array)                \
    do {                                                 \
        bool ok;                                         \
        array_try_resize_with(allocator, array, 0, &ok); \
        (void)ok;                                        \
    } while (0)

struct string {
    char *items;
    usize cap, len;
};

struct string_view {
    const char *items;
    usize len;
};

// Exclusive
#define slice_init(slice, ptr, length, begin, end)    \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        usize _b, _e;                                 \
        _b = (begin) > (length) ? (length) : (begin); \
        _e = (end) > (length) ? (length) : (end);     \
        (slice)->items = (ptr) + (_b);                \
        (slice)->len = ((_e) - (_b));                 \
    } while (0)

#define slice_from_array(slice, array, begin, end) slice_init(slice, (array)->items, (array)->len, begin, end)

#define string_fmt(s) (int)(s).len, (s).items
struct string_view sv_from(const char *cstr, usize begin, usize end);
struct string_view sv_from_string(struct string *string, usize begin, usize end);
bool sv_split_by(struct string_view *sv, const char *delims, struct string_view *out);
struct string_view sv_trim_start_by(struct string_view *sv, const char *delims);
struct string_view sv_trim_end_by(struct string_view *sv, const char *delims);
struct string_view sv_trim_by(struct string_view *sv, const char *delims);

void string_to_upper(struct string *string);
void string_to_lower(struct string *string);
struct string string_read_file(FILE *file);
struct string string_read_path(const char *path);
struct string string_printf(struct allocator *alloc, const char *fmt, ...);
#define cstr_printf(alloc, fmt, ...) string_printf(alloc, fmt, __VA_ARGS__).items

#ifdef MISC_IMPL

static bool is_delims_match(char target, const char *delims)
{
    for (usize i = 0; i < strlen(delims); i++) {
        if (delims[i] == target)
            return true;
    }
    return false;
}

struct string_view sv_trim_start_by(struct string_view *sv, const char *delims)
{
    struct string_view result = {0};
    usize i = 0;

    if (sv->len < 1)
        return result;

    while (i < sv->len && is_delims_match(sv->items[i], delims))
        i++;

    result.items = sv->items + i;
    result.len = sv->len - i;
    return result;
}

struct string_view sv_trim_end_by(struct string_view *sv, const char *delims)
{
    struct string_view result = {0};
    usize i;

    if (sv->len < 1)
        return result;

    i = sv->len - 1;
    while (i > 0 && is_delims_match(sv->items[i], delims))
        i--;

    result.items = sv->items;
    if (i == 0)
        result.len = 0;
    else
        result.len = i + 1;

    return result;
}

struct string_view sv_trim_by(struct string_view *sv, const char *delims)
{
    struct string_view result = sv_trim_start_by(sv, delims);
    return sv_trim_end_by(&result, delims);
}

bool sv_split_by(
    struct string_view *sv,
    const char *delims,
    struct string_view *out)
{
    struct string_view result;
    usize i = 0;

    if (sv->len == 0)
        return false;

    while (i < sv->len && !is_delims_match(sv->items[i], delims))
        i += 1;

    result.items = sv->items,
    result.len = i;

    if (i < sv->len) {
        sv->len -= i + 1;
        sv->items += i + 1;
    } else {
        sv->len = 0;
        sv->items += i;
    }

    if (out != NULL)
        *out = result;

    return true;
}

struct string_view sv_from(
    const char *cstr,
    usize begin,
    usize end)
{
    struct string_view ref = {0};
    usize len;

    if (cstr == NULL || end < begin)
        return ref;

    len = strlen(cstr);
    slice_init(&ref, cstr, len, begin, end);
    return ref;
}

void string_to_upper(struct string *string)
{
    for (usize i = 0; i < string->len; i++) {
        if (islower(string->items[i]))
            string->items[i] = toupper(string->items[i]);
    }
}

void string_to_lower(struct string *string)
{
    for (usize i = 0; i < string->len; i++)
        if (isupper(string->items[i]))
            string->items[i] = tolower(string->items[i]);
}

struct string string_read_file(FILE *file)
{
    struct string string = {0};
    long pos;

    if (feof(file))
        return string;

    if (fseek(file, 0, SEEK_END) != 0)
        return string;

    if ((pos = ftell(file)) <= 0)
        return string;

    rewind(file);
    array_resize(&string, (usize)pos + 1);
    fread(string.items, 1, string.cap, file);
    string.len = (usize)pos;
    return string;
}

struct string string_read_path(const char *path)
{
    struct string result = {0};
    FILE *file = fopen(path, "r");

    if (file != NULL) {
        result = string_read_file(file);
        fclose(file);
    }
    return result;
}

struct string_view sv_from_string(
    struct string *str,
    usize begin,
    usize end)
{
    struct string_view ref = {0};
    slice_from_array(&ref, str, begin, end);
    return ref;
}

struct string string_printf(struct allocator *alloc, const char *fmt, ...)
{
    struct string str = {0};
    va_list va;
    int size;

    va_start(va, fmt);
    size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        array_resize_with(alloc, &str, (usize)size + 1);
        va_start(va, fmt);
        vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size;
    }

    return str;
}
#endif

#define MISC_FNV_BASIS (0x811c9dc5)
#define MISC_FNV_PRIME (0x01000193)

#ifndef MISC_MAP_LOADF
#define MISC_MAP_LOADF (0.55)
#else
#if MISC_MAP_LOADF >= 1.0
#error Load factor must be less than 1.0
#endif
#endif

#ifndef MISC_MAP_MINIMUM
#define MISC_MAP_MINIMUM (8)
#endif

/*

A proper Hash map with open addressing, inspired from the book
`Crafting interpreters`.

This hashmap uses void* as a mean to do generic.
Let K and V be a type of key and value respectively.
On such function, one must provide a K* and V* as an argument,
each with the size.

K* and V* will be DEEP COPIED from caller into the table.
If user trying to get a value from a said table, the user must NOT
at ANY circumstances, calling free() on the return value directly.
That would resulting in double free after the call of map_free().

K* and V* value from the table will be freed when:
1. Call of map_delete()
2. Call of map_free()

map_free() will free all the backing memory of K* and V* thoroughly
until N capacity of table. map_delete() will only free
1 entry of K and V, marking it as tombstone and can be used again
if needed.

*/

struct hm_entry {
    void *key, *value;
    u32 hash, key_size;
};

struct hm_pair {
    const void *key, *value;
    usize key_size, pos;
};

struct hash_map {
    struct hm_entry *items;
    usize cap, len;
};

u32 misc_fnv1a(const void *ptr, usize size);

bool hm_init_with(struct allocator *alloc, struct hash_map *map, usize capacity);
bool hm_put_with(struct allocator *alloc, struct hash_map *map, const void *key, usize key_size, const void *value, usize value_size);
bool hm_delete_with(struct allocator *alloc, struct hash_map *map, const void *key, usize key_size);
void hm_free_with(struct allocator *alloc, struct hash_map *map);

#define hm_init(map, capacity) hm_init_with(libc_alloc, map, capacity)
#define hm_put(map, key, key_size, value, value_size) hm_put_with(libc_alloc, map, key, key_size, value, value_size)
#define hm_delete(map, key, key_size) hm_delete_with(libc_alloc, map, key, key_size)
#define hm_free(map) hm_free_with(libc_alloc, map)

void *hm_get(struct hash_map *map, const void *key, usize key_size);
bool hm_iterate(struct hash_map *map, struct hm_pair *input);

#ifdef MISC_IMPL
#define hm_load_factor(map) ((f64)(map)->len / (f64)(map)->cap)

bool hm_init_with(
    struct allocator *alloc,
    struct hash_map *map,
    usize capacity)
{
    bool ok;
    array_try_resize_with(alloc, map, misc_align_up(capacity), &ok);
    return ok;
}

static inline bool compare_key(
    struct hm_entry *dst,
    const void *key,
    usize key_size,
    u32 hash)
{
    return dst->key_size == key_size && dst->hash == hash && memcmp(dst->key, key, key_size) == 0;
}

static struct hm_entry *hme_find(
    struct hash_map *map,
    const void *key,
    usize key_size,
    u32 hash)
{
    usize idx = hash % map->cap;
    struct hm_entry *tombstone = NULL;

    while (true) {
        struct hm_entry *entry = &map->items[idx];
        if (entry->key == NULL) {
            if (entry->value == NULL) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL)
                    tombstone = entry;
            }
        } else if (compare_key(entry, key, key_size, hash)) {
            return entry;
        }
        idx = (idx + 1) % map->cap;
    }
}

static bool hm_grow(struct allocator *alloc, struct hash_map *map, usize into)
{
    struct hash_map newer = {0};
    bool ok;

    array_try_resize_with(alloc, &newer, into, &ok);
    if (!ok)
        return ok;

    for (usize i = 0; i < map->cap; i++) {
        struct hm_entry *entry, *dest;
        entry = &map->items[i];

        if (entry->key == NULL)
            continue;

        dest = hme_find(&newer, entry->key, entry->key_size, entry->hash);
        *dest = *entry;
        newer.len += 1;
    }

    array_free_with(alloc, map);
    *map = newer;
    return true;
}

bool hm_put_with(
    struct allocator *alloc,
    struct hash_map *map,
    const void *key,
    usize key_size,
    const void *value,
    usize value_size)
{
    struct hm_entry *entry;
    u32 hash;
    bool is_new_key, is_new_pull, ret = false;

    if (map->cap < MISC_MAP_MINIMUM) {
        if (!hm_init_with(alloc, map, MISC_MAP_MINIMUM))
            goto end;

    }

    if (hm_load_factor(map) >= MISC_MAP_LOADF) {
        if (!hm_grow(alloc, map, map->cap * 2))
            goto end;
    }

    hash = misc_fnv1a(key, key_size);
    entry = hme_find(map, key, key_size, hash);
    is_new_key = entry->key == NULL;
    is_new_pull = is_new_key && entry->value == NULL;

    if (is_new_key) {
        if ((entry->key = alloc->alloc(alloc->any, key_size, 1)) != NULL) {
            if ((entry->value = alloc->alloc(alloc->any, key_size, 1)) == NULL) {
                alloc->free(alloc->any, entry->key);
                goto end;
            }
        } else goto end;

        entry->key_size = key_size;
        entry->hash = hash;
        memmove(entry->key, key, key_size);

        if (is_new_pull) map->len++;
    }

    memmove(entry->value, value, value_size);
    ret = true;

end:
    return ret;
}

void *hm_get(
    struct hash_map *map,
    const void *key,
    usize key_size)
{
    struct hm_entry *entry = hme_find(map, key, key_size, misc_fnv1a(key, key_size));
    if (entry->key != NULL)
        return entry->value;

    return NULL;
}

bool hm_delete_with(
    struct allocator *alloc,
    struct hash_map *map,
    const void *key,
    usize key_size)
{
    struct hm_entry *entry = hme_find(map, key, key_size, misc_fnv1a(key, key_size));
    if (entry->key == NULL)
        return false;

    alloc->free(alloc->any, entry->key);
    alloc->free(alloc->any, entry->value);
    memset(entry, 0, sizeof *entry);
    entry->value = (void*)0xdead;
    map->len--;
    return true;
}

void hm_free_with(struct allocator *alloc, struct hash_map *map)
{
    for (usize i = 0; i < map->cap; i++) {
        struct hm_entry entry = map->items[i];
        if (entry.key == NULL || (uintptr_t)entry.value == 0xdead)
            continue;

        alloc->free(alloc->any, entry.key);
        alloc->free(alloc->any, entry.value);
    }
    array_free_with(alloc, map);
}

bool hm_iterate(struct hash_map *map, struct hm_pair *input)
{
    for (; input->pos < map->cap; input->pos++) {
        struct hm_entry entry = map->items[input->pos];
        if (entry.key != NULL) {
            input->key = entry.key;
            input->value = entry.value;
            input->key_size = entry.key_size;
            input->pos++;
            return true;
        }
    }

    input->pos = 0;
    return false;
}

u32 misc_fnv1a(const void *ptr, usize size)
{
    const u8 *bytes = ptr;
    u32 base_val = MISC_FNV_BASIS;
    for (usize i = 0; i < size; i++) {
        base_val ^= bytes[i];
        base_val = (base_val * MISC_FNV_PRIME) & 0xFFFFFFFF;
    }
    return base_val;
}

#endif

/*

Ring buffer, Circular buffer, Cyclic buffer.
This is a wrapper around fixed-size buffer that let you
read/write at a specific position without worried about
doing it past its size, because if it does, it'll wrap
around to position 0 instead of going past the size of buffer.

[ H, e, l, l, o, 0x0, 0x0, 0x0 ]
  ↑               ↑
 read position   write position

[ l, d, !, \n, 0x0, W, o, r ]
                    ↑
                read/write position

*/

struct ring_buffer {
    void *buffer;
    usize write_pos, read_pos, len;
};

struct ring_buffer rb_init(void *buffer /* assume aligned */, usize len);
usize rb_write(struct ring_buffer *rb, const void *src, usize len);
usize rb_read(struct ring_buffer *rb, void *dst, usize len);
void rb_seek_write(struct ring_buffer *rb, isize len, int whence);
void rb_seek_read(struct ring_buffer *rb, isize len, int whence);
void rb_clear(struct ring_buffer *rb);

#ifdef MISC_IMPL
struct ring_buffer rb_init(void *buffer, usize len)
{
    return (struct ring_buffer) {
        .buffer = buffer,
        .len = len,
        .write_pos = 0,
        .read_pos = 0,
    };
}

usize rb_write(struct ring_buffer *rb, const void *src, usize len)
{
    const u8 *repr = src;
    u8 *buf = rb->buffer;
    usize i;

    for (i = 0; i < len; i++, rb->write_pos = (rb->write_pos + 1) % rb->len)
        buf[rb->write_pos] = repr[i];

    return i;
}

usize rb_read(struct ring_buffer *rb, void *dst, usize len)
{
    u8 *repr = dst;
    const u8 *buf = rb->buffer;
    usize i;

    for (i = 0;
        i < len;
        i++, rb->read_pos = (rb->read_pos + 1) % rb->len)
    {
        repr[i] = buf[rb->read_pos];
    }

    return i;
}

void rb_seek_write(struct ring_buffer *rb, isize len, int whence)
{
    switch (whence) {
    case SEEK_SET:
        len %= rb->len;
        break;

    case SEEK_CUR:
        len = (len + (isize)rb->write_pos) % rb->len;
        break;

    case SEEK_END:
        len = ((isize)rb->len - len) % rb->len;
        break;

    default:
        return;
    }

    rb->write_pos = (usize)len;
}

void rb_seek_read(struct ring_buffer *rb, isize len, int whence)
{
    switch (whence) {
    case SEEK_SET:
        len %= rb->len;
        break;

    case SEEK_CUR:
        len = (len + (isize)rb->read_pos) % rb->len;
        break;

    case SEEK_END:
        len = ((isize)rb->len - len) % rb->len;
        break;

    default:
        return;
    }

    rb->read_pos = (usize)len;
}

void rb_clear(struct ring_buffer *rb)
{
    memset(rb->buffer, 0, rb->len);
    rb->write_pos = 0;
    rb->read_pos = 0;
}

#endif

#endif
