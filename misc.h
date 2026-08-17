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
typedef size_t usize;
typedef intptr_t isize;
typedef float f32;
typedef double f64;

#define MISC_ALIGN (sizeof(void*))
#define fprintfn(f, fmt, ...) fprintf(f, fmt "\n", __VA_ARGS__)
#define printfn(fmt, ...) fprintfn(stdout, fmt, __VA_ARGS__)

/*

Common interface to allocate memory in a uniform manner.
@any could be anything, yes, ANYTHING.
@size is a size, of course.
@alignment must be a power of 2, if not, the creator must explicitly return NULL.
@alignment is then packed into @size and call allocation API, so the allocated
memory is aligned. So the alignment is applied before and not after it's allocated.

This header will expose 2 kind of allocators:
1. Libc allocator as @misc_libc_alloc.
2. System memory mapped virtual memory as @misc_mmap_alloc.

For most of the time, you should use the @misc_libc_alloc, as it will uses
the malloc(3), realloc(3) and free(3) with the addition of an alignment in it.

the implemented allocators here will store additional address metadata such as its size
in allocated memory (or a fat pointer).
Maybe we could make it store the alignment, so we can get more clean function table here haha.

Note that it's not thread safe since c99 doesn't have the threadlocal yet, so one must use a lock mechanism.
Functions that use it will have *_with postfix in their name.
If one function use one custom allocator, all of the other functions must uses the same allocator, MUST!

*/
typedef struct {
    void* any;
    void* (*allocate)(void* any, usize size, usize alignment);
    void* (*reallocate)(void* any, void* ptr, usize size, usize alignment);
    void (*deallocate)(void* any, void* ptr);
} GeneralAllocator;

// Use this for most of the time
extern GeneralAllocator* const misc_libc_alloc;

// You'll rarely use this anyway
extern GeneralAllocator* const misc_mmap_alloc;

#define misc_palign(ptr, align) ((void*)(((uintptr_t)(ptr) + (align) - 1) & ~((align) - 1)))
#define misc_align_up(size) ((uintptr_t)misc_palign(size, 8))

void* misc_mmap(usize size, usize alignment);
void* misc_remap(void* ptr, usize size, usize alignment);
void misc_unmap(void* ptr);

#ifdef __unix__
#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES > 0
#define MISC_POSIX_MAP
#define MISC_MAP_ERR MAP_FAILED
#else
#define MISC_MAP_ERR NULL
#endif

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define MISC_MAP_ERR NULL
#define MISC_WINAPI

#else
#define MISC_MAP_ERR NULL
#endif

#ifdef MISC_IMPL

void* misc_alloc(void* any, usize size, usize alignment)
{
    (void)any;
    return malloc((uintptr_t)misc_palign(size, alignment));
}

void* misc_realloc(void* any, void* ptr, usize size, usize alignment)
{
    (void)any;
    return realloc(ptr, (uintptr_t)misc_palign(size, alignment));
}

void misc_free(void* any, void* ptr)
{
    (void)any;
    free(ptr);
}

static GeneralAllocator libc_alloc = {
    .allocate = misc_alloc,
    .reallocate = misc_realloc,
    .deallocate = misc_free,
};

GeneralAllocator* const misc_libc_alloc = &libc_alloc;

void* misc_mmap(usize size, usize alignment)
{
#ifdef MISC_WINAPI
    HANDLE handle;
#endif

    void* ptr;

    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
        return NULL;

#ifdef MISC_POSIX_MAP
    size = (usize)(uintptr_t)misc_palign(size + sizeof size, alignment);
    ptr = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);

#elif defined(MISC_WINAPI)
    size = (usize)(uintptr_t)misc_palign(size + sizeof handle + sizeof size, alignment);
    handle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        NULL);

    if (handle == NULL)
        return NULL;

    ptr = MapViewOfFile(
        handle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size);
#else
    ptr = malloc(misc_palign(size, alignment));

#endif

    if (ptr == MISC_MAP_ERR) {
#ifdef MISC_WINAPI
        CloseHandle(handle);
#endif
        return NULL;
    }

#ifdef MISC_POSIX_MAP
    memmove(ptr, &size, sizeof size);
    ptr = (u8*)ptr + sizeof size;

#elif defined(MISC_WINAPI)
    memmove(ptr, handle, sizeof handle);
    memmove((u8*)ptr + sizeof handle, &size, sizeof size);
    ptr = (u8*)ptr + sizeof handle + sizeof size;

#endif

    return ptr;
}

void* misc_remap(void* ptr, usize size, usize alignment)
{
    void* newer;

    if (size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
        return NULL;

    if (ptr == NULL)
        return misc_mmap(size, alignment);

#if defined(MISC_POSIX_MAP) || defined(MISC_WINAPI)
    usize* true_size = (void*)((u8*)ptr - sizeof(usize));
    usize copied = *true_size > size ? size : *true_size;
    newer = misc_mmap(size, alignment);

    if (newer == NULL)
        return NULL;

    memmove(newer, ptr, copied);
    misc_unmap(ptr);

#else
    newer = realloc(ptr, misc_palign(size, alignment));
#endif

    return newer;
}

void misc_unmap(void* ptr)
{
    if (ptr == NULL)
        return;
#ifdef MISC_POSIX_MAP
    ptr = (u8*)ptr - sizeof(usize);
    usize* true_size = ptr;
    munmap(ptr, *true_size);

#elif defined(MISC_WINAPI)
    ptr = (u8*)ptr - sizeof(usize) - sizeof(HANDLE);
    UnmapViewOfFile(ptr);

#else
    free(ptr);
#endif
}

void* _misc_mmap(void* any, usize size, usize alignment)
{
    (void)any;
    return misc_mmap(size, alignment);
}

void* _misc_remap(void* any, void* ptr, usize size, usize alignment)
{
    (void)any;
    return misc_remap(ptr, size, alignment);
}

void _misc_unmap(void* any, void* ptr)
{
    (void)any;
    misc_unmap(ptr);
}

static GeneralAllocator mmap_alloc = {
    .allocate = _misc_mmap,
    .reallocate = _misc_remap,
    .deallocate = _misc_unmap,
};

GeneralAllocator* const misc_mmap_alloc = &mmap_alloc;

#endif

void* misc_strict_alloc(usize size);
void* misc_strict_realloc(void* ptr, usize size);

#define misc_stacked(T, ...) (&(T) { __VA_ARGS__ })
#define misc_heaped(T, ...) memmove(misc_strict_alloc(sizeof(T)), misc_stacked(T, __VA_ARGS__), sizeof(T))

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

#ifdef MISC_IMPL
void* misc_strict_alloc(usize size)
{
    void* p = malloc(size);
    misc_assert(p != NULL, "malloc() returns null");
    return p;
}

void* misc_strict_realloc(void* ptr, usize size)
{
    void* p = realloc(ptr, size);
    misc_assert(p != NULL, "realloc() returns null");
    return p;
}
#endif

typedef struct NodeLink NodeLink;
struct NodeLink {
    NodeLink* next;
    // ...
};

NodeLink* nl_init_with(GeneralAllocator* allocator, usize size);
NodeLink* nl_put_after_with(GeneralAllocator* allocator, NodeLink* node, usize size);
NodeLink* nl_put_before_with(GeneralAllocator* allocator, NodeLink* node, usize size);
void nl_free_with(GeneralAllocator* allocator, NodeLink* node);

NodeLink* nl_init(usize size);
NodeLink* nl_put_after(NodeLink* node, usize size);
NodeLink* nl_put_before(NodeLink* node, usize size);
NodeLink* nl_pop_after(NodeLink* node);
NodeLink* nl_last(NodeLink* node);
usize nl_len(NodeLink* node);
void* nl_value(NodeLink* node);
void nl_free(NodeLink* node);

#ifdef MISC_IMPL
NodeLink* nl_init_with(GeneralAllocator* allocator, usize size)
{
    NodeLink* node = allocator->allocate(allocator->any, size, MISC_ALIGN);
    if (node != NULL)
        node->next = NULL;

    return node;
}

NodeLink* nl_put_after_with(GeneralAllocator* allocator, NodeLink* node, usize size)
{
    NodeLink* next = nl_init_with(allocator, size);
    node->next = next;
    return next;
}

NodeLink* nl_put_before_with(GeneralAllocator* allocator, NodeLink* node, usize size)
{
    NodeLink* before = nl_init_with(allocator, size);
    if (before != NULL)
        before->next = node;

    return before;
}

void nl_free_with(GeneralAllocator* allocator, NodeLink* node)
{
    while (node != NULL) {
        NodeLink* tmp = node->next;
        allocator->deallocate(allocator->any, node);
        node = tmp;
    }
}

NodeLink* nl_init(usize size)
{
    return nl_init_with(misc_libc_alloc, size);
}

NodeLink* nl_put_after(NodeLink* node, usize size)
{
    return nl_put_after_with(NULL, node, size);
}

NodeLink* nl_put_before(NodeLink* node, usize size)
{
    return nl_put_before_with(NULL, node, size);
}

NodeLink* nl_pop_after(NodeLink* node)
{
    NodeLink* next = node->next;
    NodeLink* tmp = next != NULL ? next->next : NULL;
    node->next = tmp;
    return next;
}

NodeLink* nl_last(NodeLink* node)
{
    if (node == NULL)
        return NULL;
    while (node->next != NULL)
        node = node->next;

    return node;
}

void* nl_value(NodeLink* node)
{
    /*
    32-bit: size 4
    64-bit: size 8

    Will it break the alignment? definitely not.
    Unless, the node itself had an odd alignment,
    which is not my problem.
    */
    return (u8*)node + sizeof *node;
}

usize nl_len(NodeLink* node)
{
    usize count = 0;
    while (node != NULL)
        node = node->next,
        count++;

    return count;
}

void nl_free(NodeLink* node)
{
    nl_free_with(misc_libc_alloc, node);
}
#endif

typedef struct Arena Arena;

Arena* arena_init_with(GeneralAllocator* allocator, usize size);
void* arena_alloc_with(GeneralAllocator* allocator, Arena* arena, usize size);
void* arena_realloc_with(GeneralAllocator* allocator, Arena* arena, void* ptr, usize size_before, usize size_after);
void arena_free_with(GeneralAllocator* allocator, Arena* arena);

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

Arena* arena_init_with(GeneralAllocator* allocator, usize size)
{
    if (size < 1)
        return NULL;

    Arena* arena = allocator->allocate(allocator->any, sizeof *arena, MISC_ALIGN);
    if (arena == NULL)
        return NULL;

    ArenaBody body = { .cap = size };
    arena->head = nl_init_with(allocator, sizeof body + size);
    if (arena->head == NULL) {
        allocator->deallocate(allocator->any, arena);
        return NULL;
    }

    arena->last = arena->head;
    ArenaBody* value = nl_value(arena->head);
    *value = body;
    return arena;
}

void* arena_alloc_with(
    GeneralAllocator* allocator,
    Arena* arena,
    usize size)
{
    if (arena == NULL || size < 1)
        return NULL;

    NodeLink* last = arena->last;
    ArenaBody* body = nl_value(last);

    /*
    Make @size divisible by the host default alignment.
    On 64-bit, that would be 8 bytes, while 32-bit is 4 bytes.

    This is important, if the caller provide the size for @arena_init
    that is considered odd or misaligned by the OS, the @size here will
    make that irrelevant, since if it doesn't had enough capacity by the
    @size + (additional to make the @size aligned), it will create a new
    arena (on the next linked list, for sure), and will use that, so the
    odd aligned part of the previous arena will never used.

    You can comment this and see what happen. I'm using zig compiler to compile
    the example, and it absolutely blew up my terminal with stack trace because
    the program trying to access memory that is not aligned. (fuckin learned it the hard way dawg✌️😭)
    */
    size = misc_align_up(size);

    if (body->cap - body->len < size) {
        usize new_size = (body->cap > size ? body->cap : size) + size;
        ArenaBody newer = { .cap = new_size };
        NodeLink* new_tail = nl_put_after_with(allocator, last, sizeof newer + new_size);
        if (new_tail == NULL)
            return NULL;

        arena->last = new_tail;
        last = arena->last;
        body = nl_value(last);
        *body = newer;
    }

    void* ptr = (u8*)body + sizeof *body + body->len;
    body->len += size;
    return ptr;
}

void* arena_realloc_with(
    GeneralAllocator* allocator,
    Arena* arena,
    void* ptr,
    usize size_before,
    usize size_after)
{
    if (arena == NULL || size_after == 0)
        return NULL;

    void* newer = arena_alloc_with(allocator, arena, size_after);
    if (ptr == NULL)
        return newer;

    usize true_size = size_before > size_after ? size_after : size_before;
    return memmove(newer, ptr, true_size);
}

void arena_free_with(GeneralAllocator* allocator, Arena* arena)
{
    if (arena != NULL) {
        nl_free_with(allocator, arena->head);
        allocator->deallocate(allocator->any, arena);
    }
}

Arena* arena_init(usize size)
{
    return arena_init_with(misc_libc_alloc, size);
}

void* arena_alloc(Arena* arena, usize size)
{
    return arena_alloc_with(misc_libc_alloc, arena, size);
}

void* arena_realloc(
    Arena* arena,
    void* ptr,
    usize size_before,
    usize size_after)
{
    return arena_realloc_with(misc_libc_alloc, arena, ptr, size_before, size_after);
}

void arena_free(Arena* arena)
{
    arena_free_with(misc_libc_alloc, arena);
}

usize arena_size(Arena* arena)
{
    usize size = 0;
    if (arena == NULL)
        return size;

    NodeLink* node = arena->head;
    while (node != NULL) {
        ArenaBody* body = nl_value(node);
        size += body->cap;
        node = node->next;
    }
    return size;
}
#endif

#define MISC_ARRAY_RESERVE (8)

#define Array(T)   \
    struct {       \
        T* items;  \
        usize cap; \
        usize len; \
    }

#define array_is_empty(array) ((array) != NULL ? ((array)->items == NULL || (array)->cap < 1) : 1)
#define array_remains(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define array_try_resize_with(allocator, array, N, ok)                                                                     \
    do {                                                                                                                   \
        if ((N) <= 0) {                                                                                                    \
            (allocator)->deallocate((allocator)->any, (array)->items);                                                     \
            (array)->items = NULL;                                                                                         \
            (array)->cap = 0;                                                                                              \
            (array)->len = 0;                                                                                              \
            *(ok) = 1;                                                                                                     \
        } else {                                                                                                           \
            void* tmp;                                                                                                     \
            if ((array)->cap == 0) {                                                                                       \
                tmp = (allocator)->allocate((allocator)->any, (N) * sizeof *(array)->items, MISC_ALIGN);                   \
            } else {                                                                                                       \
                tmp = (allocator)->reallocate((allocator)->any, (array)->items, (N) * sizeof *(array)->items, MISC_ALIGN); \
            }                                                                                                              \
            if (tmp != NULL) {                                                                                             \
                *(ok) = 1;                                                                                                 \
                (array)->items = tmp;                                                                                      \
                (array)->cap = (N);                                                                                        \
                if ((N) < (array)->len) {                                                                                  \
                    (array)->len = (N);                                                                                    \
                }                                                                                                          \
            } else {                                                                                                       \
                *(ok) = 0;                                                                                                 \
            }                                                                                                              \
        }                                                                                                                  \
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

#define array_try_extend_with(allocator, array, many_ptr, N, ok)                                \
    do {                                                                                        \
        if ((many_ptr) != NULL && (N) > 0) {                                                    \
            if (array_is_empty(array) || array_remains(array) <= (N)) {                         \
                array_try_resize(allocatr, array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok); \
                if (!*(ok)) {                                                                   \
                    break;                                                                      \
                }                                                                               \
            }                                                                                   \
            memmove((array)->items + (array)->len, (many_ptr), (N) * sizeof *(array)->items);   \
            (array)->len += (N);                                                                \
            *(ok) = 1;                                                                          \
        } else {                                                                                \
            *(ok) = 0;                                                                          \
        }                                                                                       \
    } while (0)

#define array_try_resize(array, N, ok) array_try_resize_with(misc_libc_alloc, array, N, ok)
#define array_try_append(array, item, ok) array_try_append_with(misc_libc_alloc, array, item, ok)
#define array_try_extend(array, items, N, ok) array_try_extend_with(misc_libc_alloc, array, items, N, ok)

#define array_resize(array, N)                    \
    do {                                          \
        bool ok;                                  \
        array_try_resize(array, N, &ok);          \
        misc_assert(ok, "array_append() failed"); \
    } while (0)

#define array_append(array, item)                 \
    do {                                          \
        bool ok;                                  \
        array_try_append(array, item, &ok);       \
        misc_assert(ok, "array_append() failed"); \
    } while (0)

#define array_extend(array, many_ptr, N)           \
    do {                                           \
        bool ok;                                   \
        array_try_extend(array, many_ptr, N, &ok); \
        misc_assert(ok, "array_extend() failed");  \
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

#define array_try_append_at(array, idx, item, ok) array_try_append_at_with(misc_libc_alloc, array, idx, item, ok)
#define array_append_at(array, idx, item)                \
    do {                                                 \
        bool ok = false;                                 \
        array_try_append_at(array, idx, item, &ok);      \
        misc_assert(ok, "array_try_append_at() failed"); \
    } while (0)

#define array_make_fit_with(allocator, array)                       \
    do {                                                            \
        bool ok;                                                    \
        array_try_resize_with(allocator, array, (array)->len, &ok); \
        (void)ok;                                                   \
    } while (0)

#define array_make_fit(array) array_make_fit_with(misc_libc_alloc, array)

#define array_free_with(allocator, array)                \
    do {                                                 \
        bool ok;                                         \
        array_try_resize_with(allocator, array, 0, &ok); \
        (void)ok;                                        \
    } while (0)

#define array_free(array) array_free_with(misc_libc_alloc, array)

#define Slice(T)        \
    struct {            \
        const T* items; \
        usize len;      \
    }

typedef Array(char) String;
typedef Slice(char) StringView;

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
StringView sv_from(const char* cstr, usize begin, usize end);
StringView sv_from_string(String* string, usize begin, usize end);
bool sv_split_by(StringView* sv, const char* delims, StringView* out);
StringView sv_trim_start_by(StringView* sv, const char* delims);
StringView sv_trim_end_by(StringView* sv, const char* delims);
StringView sv_trim_by(StringView* sv, const char* delims);
void string_to_upper(String* string);
void string_to_lower(String* string);
String string_printf(const char* fmt, ...);
String string_read_file(FILE* file);
String string_read_path(const char* path);
char* cstr_arena_printf(GeneralAllocator* allocator, Arena* arena, const char* fmt, ...);
char* cstr_printf(const char* fmt, ...);

#ifdef MISC_IMPL

static bool is_delims_match(char target, const char* delims)
{
    for (usize i = 0; i < strlen(delims); i++) {
        if (delims[i] == target)
            return true;
    }
    return false;
}

StringView sv_trim_start_by(StringView* sv, const char* delims)
{
    StringView result = { 0 };
    if (sv->len < 1)
        return result;

    usize i = 0;
    while (i < sv->len && is_delims_match(sv->items[i], delims))
        i++;

    result.items = sv->items + i;
    result.len = sv->len - i;
    return result;
}

StringView sv_trim_end_by(StringView* sv, const char* delims)
{
    StringView result = { 0 };
    if (sv->len < 1)
        return result;

    usize i = sv->len - 1;
    while (i > 0 && is_delims_match(sv->items[i], delims))
        i--;

    result.items = sv->items;
    if (i == 0)
        result.len = 0;
    else
        result.len = i + 1;
    return result;
}

StringView sv_trim_by(StringView* sv, const char* delims)
{
    StringView result = sv_trim_start_by(sv, delims);
    return sv_trim_end_by(&result, delims);
}

bool sv_split_by(
    StringView* sv,
    const char* delims,
    StringView* out)
{
    if (sv->len == 0)
        return false;

    usize i = 0;
    while (i < sv->len && !is_delims_match(sv->items[i], delims))
        i += 1;

    StringView result = {
        .items = sv->items,
        .len = i,
    };

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

StringView sv_from(
    const char* cstr,
    usize begin,
    usize end)
{
    StringView ref = { 0 };
    if (cstr == NULL || end < begin)
        return ref;

    usize len = strlen(cstr);
    slice_init(&ref, cstr, len, begin, end);
    return ref;
}

void string_to_upper(String* string)
{
    for (usize i = 0; i < string->len; i++) {
        if (islower(string->items[i]))
            string->items[i] = toupper(string->items[i]);
    }
}

void string_to_lower(String* string)
{
    for (usize i = 0; i < string->len; i++)
        if (isupper(string->items[i]))
            string->items[i] = tolower(string->items[i]);
}

String string_read_file(FILE* file)
{
    String string = { 0 };
    if (feof(file))
        return string;

    long pos;
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

String string_read_path(const char* path)
{
    String result = { 0 };
    FILE* file = fopen(path, "r");
    if (file != NULL) {
        result = string_read_file(file);
        fclose(file);
    }
    return result;
}

StringView sv_from_string(
    String* str,
    usize begin,
    usize end)
{
    StringView ref = { 0 };
    slice_from_array(&ref, str, begin, end);
    return ref;
}

char* cstr_arena_printf(
    GeneralAllocator* allocator,
    Arena* arena,
    const char* fmt,
    ...)
{
    va_list va;
    char* buf = NULL;
    int size = 0;

    va_start(va, fmt);
    size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        buf = allocator == NULL ? arena_alloc(arena, (usize)size + 1) : arena_alloc_with(allocator, arena, (usize)size + 1);
        va_start(va, fmt);
        vsnprintf(buf, (usize)size + 1, fmt, va);
        va_end(va);
    }

    return buf;
}

char* cstr_printf(const char* fmt, ...)
{
    va_list va;
    char* buf = NULL;
    int size = 0;

    va_start(va, fmt);
    size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        buf = misc_strict_alloc((usize)size + 1);
        va_start(va, fmt);
        vsnprintf(buf, (usize)size + 1, fmt, va);
        va_end(va);
    }

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
        array_resize(&str, (usize)size + 1);
        va_start(va, fmt);
        vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size;
    }

    return str;
}
#endif

#define MISC_FNV_BASIS (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME (0x100000001b3ULL)

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

typedef struct {
    void* key;
    void* value;
    u64 hash;
    usize key_size;
} MapEntry;

typedef struct {
    const void* key;
    const void* value;
    usize key_size;
    usize pos;
} MapKV;

typedef struct {
    MapEntry* items;
    usize cap;
    usize len;
} Map;

u64 misc_fnv1a(const void* ptr, usize size);
void map_init(Map* map);
void map_put(Map* map, const void* key, usize key_size, const void* value, usize value_size);
void* map_get(Map* map, const void* key, usize key_size);
void map_delete(Map* map, const void* key, usize key_size);
bool map_iterate(Map* map, MapKV* input);
void map_free(Map* map);

#ifdef MISC_IMPL
#define map_load_factor(map) ((f64)(map)->len / (f64)(map)->cap)

void map_init(Map* map)
{
    array_resize(map, MISC_MAP_MINIMUM);
}

static bool compare_key(
    MapEntry* dst,
    const void* key,
    usize key_size,
    u64 hash)
{
    return dst->key_size == key_size && dst->hash == hash && memcmp(dst->key, key, key_size) == 0;
}

static MapEntry* me_find(
    Map* map,
    const void* key,
    usize key_size,
    u64 hash)
{
    usize idx = hash & (map->cap - 1);
    MapEntry* tombstone = NULL;

    while (true) {
        MapEntry* entry = &map->items[idx];
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
        idx = (idx + 1) & (map->cap - 1);
    }
}

static void map_grow(Map* map, usize into)
{
    Map newer = { 0 };
    array_resize(&newer, into);
    newer.len = map->len;

    for (usize i = 0; i < map->cap; i++) {
        MapEntry* entry = &map->items[i];
        if (entry->key == NULL)
            continue;

        MapEntry* dest = me_find(&newer, entry->key, entry->key_size, entry->hash);
        *dest = *entry;
    }

    array_free(map);
    *map = newer;
}

void map_put(
    Map* map,
    const void* key,
    usize key_size,
    const void* value,
    usize value_size)
{
    if (map->cap < MISC_MAP_MINIMUM) {
        map_init(map);
    } else if (map_load_factor(map) >= MISC_MAP_LOADF) {
        map_grow(map, map->cap * 2);
    }

    u64 hash = misc_fnv1a(key, key_size);
    MapEntry* entry = me_find(map, key, key_size, hash);
    bool isNewKey = entry->key == NULL;
    if (isNewKey) {
        usize merge = key_size + value_size;
        usize round_up = misc_align_up(merge);
        u8* pool = misc_strict_alloc(round_up);
        entry->key = pool;
        entry->value = pool + key_size + (round_up - merge);
        entry->key_size = key_size;
        entry->hash = hash;
        memmove(entry->key, key, key_size);
        map->len++;
    }
    memmove(entry->value, value, value_size);
}

void* map_get(
    Map* map,
    const void* key,
    usize key_size)
{
    MapEntry* entry = me_find(map, key, key_size, misc_fnv1a(key, key_size));
    if (entry->key != NULL)
        return entry->value;
    return NULL;
}

void map_delete(
    Map* map,
    const void* key,
    usize key_size)
{
    MapEntry* entry = me_find(map, key, key_size, misc_fnv1a(key, key_size));
    if (entry->key == NULL)
        return;

    free(entry->key);
    memset(entry, 0, sizeof *entry);
    entry->value = (void*)0xdead;
    map->len--;
}

void map_free(Map* map)
{
    for (usize i = 0; i < map->cap; i++) {
        MapEntry entry = map->items[i];
        if (entry.key == NULL || (uintptr_t)entry.value == 0xdead)
            continue;

        free(entry.key);
    }
    array_free(map);
}

bool map_iterate(Map* map, MapKV* input)
{
    for (; input->pos < map->cap; input->pos++) {
        MapEntry entry = map->items[input->pos];
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

u64 misc_fnv1a(const void* ptr, usize size)
{
    const u8* bytes = ptr;
    u64 base_val = MISC_FNV_BASIS;
    for (u64 i = 0; i < size; i++) {
        base_val *= MISC_FNV_PRIME;
        base_val ^= bytes[i];
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

typedef struct {
    void* buffer;
    usize
        write_pos,
        read_pos,
        len;
} RingBuffer;

RingBuffer rb_init(void* buffer, usize len);
usize rb_write(RingBuffer* rb, const void* src, usize len);
usize rb_read(RingBuffer* rb, void* dst, usize len);
void rb_seek_write(RingBuffer* rb, isize len, int whence);
void rb_seek_read(RingBuffer* rb, isize len, int whence);
void rb_clear(RingBuffer* rb);

#ifdef MISC_IMPL
RingBuffer rb_init(void* buffer, usize len)
{
    return (RingBuffer) {
        .buffer = buffer,
        .len = len,
        .write_pos = 0,
        .read_pos = 0,
    };
}

usize rb_write(RingBuffer* rb, const void* src, usize len)
{
    const u8* repr = src;
    u8* buf = rb->buffer;

    usize i;
    for (i = 0; i < len; i++, rb->write_pos = (rb->write_pos + 1) % rb->len) {
        buf[rb->write_pos] = repr[i];
    }

    return i;
}

usize rb_read(RingBuffer* rb, void* dst, usize len)
{
    u8* repr = dst;
    const u8* buf = rb->buffer;

    usize i;
    for (i = 0;
        i < len;
        i++, rb->read_pos = (rb->read_pos + 1) % rb->len) {
        repr[i] = buf[rb->read_pos];
    }

    return i;
}

void rb_seek_write(RingBuffer* rb, isize len, int whence)
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

void rb_seek_read(RingBuffer* rb, isize len, int whence)
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

void rb_clear(RingBuffer* rb)
{
    memset(rb->buffer, 0, rb->len);
    rb->write_pos = 0;
    rb->read_pos = 0;
}

#endif

#endif
