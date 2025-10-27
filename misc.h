/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under MIT License. All right reserved.

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

#if defined(__LP64__) || defined(__LLP64__) || defined(_WIN64)
#define MISC_HOST_BITS 64
#elif defined(__LP32__) || defined(__LLP32__) || defined(_WIN32)
#define MISC_HOST_BITS 32
#else
#error Unknown OS bits (not 64 nor 32)
#endif

#define MISC_WORD_SIZE (sizeof(void *))

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#ifndef _LARGE_FILE
#define _LARGE_FILE
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#if __STDC_VERSION__ >= 202000L

#define MISC_SWAP(lhs, rhs)               \
    do {                                  \
        typeof((lhs)) loose_copy = (lhs); \
        (lhs) = (rhs);                    \
        (rhs) = loose_copy;               \
    } while (0)

#endif

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)

#define MISC_POSIX_HOST

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#ifdef MISC_POSIX_HOST

#include <sys/mman.h>

#define MISC_BUILTIN_ALLOC(size) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0)
#define MISC_BUILTIN_FREE(ptr, size) munmap(ptr, size)
#define MISC_BUILTIN_ALLOC_FAIL MAP_FAILED

#elif defined(_WIN32) || defined(_WIN64)

#define MISC_WINDOWS_HOST

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winbase.h>
#include <memoryapi.h>
#include <BaseTsd.h>
#include <WinDef.h>
#include <WinNT.h>


/* If you are using clang on windows, compile it with
-fms-compatibility flag. */
#if defined(_MSC_VER) || defined(__clang__)
#pragma comment(lib, "kernel32.lib")
#endif
/* Or, use -lkernel32 flag */

#define MISC_WIN32_PAGE_NAME "MISC PAGES"

#define MISC_BUILTIN_ALLOC(size) MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size)
#define MISC_BUILTIN_FREE(ptr, size) UnmapViewOfFile(ptr)
#define MISC_BUILTIN_ALLOC_FAIL NULL

#else
#error Platform unrecognized
#endif

/* ===== ARENA SECTION ===== */
#ifdef PAGE_SIZE
#define ARENA_PAGE PAGE_SIZE
#else
#define ARENA_PAGE (1ULL << 12ULL)
#endif

#define REMAIN_OF(Arena) ((Arena)->total - (Arena)->offset)

/* Arena types, a single linked list that point to the next allocator.
For a better portability, we using uint8_t* instead of raw void* on a pointer
arithmetic context.

You create the arena using arena_create() that accept 1 arguments, the initial
size.

The linked list is made of 4 members, described as follow:
1. next, is the next allocator that have 2 states, null or nonnull.
2. data, is a raw pointer from a libc allocator casted to uint8_t*.
3. total, is the total size of memory region this allocator hold.
4. offset, is the difference between base address and current address.
   This member is always incremented on a call of arena_alloc() or
   arena_realloc()

When you call arena_alloc(), it'll check if the current allocator have memory
as large as "size". If it is, it return the current memory region as a chunk of
memory

The chunk is a result of difference between base address (data) and offset,
for example:

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
   ^
 offset

Here we have a 5 worth of bytes with each of respective memory addresses.
If, say, we want 2 bytes chunk from that region, we can do:

  chunk = buffer + offset
  offset += size
  return chunk

Here, we add a difference between buffer and offset, but right now the offset
is zero, so the difference is 5. Now you have a 2 possible outcome:
1. A chunk worth of 2 bytes.
2. A chunk worth more than 2 bytes.

You could use the chunk as if it was 2 bytes of memory, or you could use it as a
chunk of 2 bytes or more, up to 0x4.

Now if you look at our allocator again, things would change

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
             ^
           offset

Ok cool, now we have 3 bytes left, starting at address 0x2.
Now, say, you want more than 2 bytes, what if 5 bytes? can you?

Remember that arena_alloc() will check if the current allocator have enough
chunk for us to take? When the remaining bytes (total - offset) is not enough,
arena_alloc() will create a new allocator, pointed by @Next

The chunk returned by arena_alloc() or arena_realloc() may be NULL, so you must
check it before using it.

Now for the best part is that we only need to free our arena's once and we're
done with it. Other neat thing is that you can use the arena and pass it around
to a bunch of function, so that you know that those section of your program need
to allocate some memory.

NOTE:
Since the arena is just a linked list, it search for a suitable arena to perform
allocation by lineary check if the arena is match the requirement (the size).
So it would be better to create a big chunk of arena. The default one is
ARENA_PAGE or 4096 bytes (same as in my system, see the getpagesize(2)), which
is enough for everyone nowadays. */

typedef struct Arena Arena;

struct Arena {
    Arena *next;
    size_t total, offset;

#ifdef MISC_WINDOWS_HOST
    HANDLE handle;
#endif

    uint8_t *data;
};

#ifdef MISC_USE_GLOBAL_ALLOCATOR
#define MISC_GLOBAL_ALLOCATOR_PAGING (ARENA_PAGE * 4)
static Arena *_misc_global_allocator = NULL;
#endif

static inline Arena *arena_create(size_t size)
{
    if (size < 1)
        return NULL;

#ifdef MISC_WINDOWS_HOST
    HANDLE handle = (HANDLE) CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        MISC_WIN32_PAGE_NAME
    );

    if (handle == NULL)
        return NULL;
#endif

    Arena *head_node = (Arena *) MISC_BUILTIN_ALLOC(sizeof *head_node + size);
    if (head_node == MISC_BUILTIN_ALLOC_FAIL)
        return NULL;

    head_node->next = NULL;
    head_node->total = size;
    head_node->offset = 0;
    head_node->data = ((uint8_t *) head_node) + sizeof *head_node;

#ifdef MISC_WINDOWS_HOST
    head_node->handle = handle;
#endif

    return head_node;
}

static inline Arena *arena_find_suitable(Arena *base_arena,
                                         size_t size,
                                         int *found)
{
    Arena *visitor = base_arena, *last_nonnull;
    while (visitor) {
        if (REMAIN_OF(visitor) >= size) {
            *found = 1;
            return visitor;
        }

        last_nonnull = visitor;
        visitor = visitor->next;
    }

    *found = 0;
    return last_nonnull;
}

static inline void *arena_alloc(Arena *input, size_t size)
{
    Arena *suitable;
    int found;
    size_t size_required;

    if (input == NULL || size < 1)
        return NULL;

    suitable = arena_find_suitable(input, size, &found);

    /*
    TODO:
    1. Special page for @size > @total
    2. Dataset member to indicating a special page
    */
    size_required = size > suitable->total ? size * 2 : suitable->total;

    if (!found) {
        suitable->next = arena_create(size_required);
        if (!suitable->next)
            return NULL;

        suitable = suitable->next;
    }

    void *result = suitable->data + suitable->offset;
    suitable->offset += size;
    return result;
}

static inline void *arena_realloc(Arena *input,
                                  void *target_ptr,
                                  size_t old_size,
                                  size_t new_size)
{
    void *result = arena_alloc(input, new_size);
    if (!result)
        return NULL;

    else if (!target_ptr)
        return result;

    memcpy(result, target_ptr, old_size > new_size ? new_size : old_size);
    return result;
}

static inline void arena_free(Arena *input)
{
    while (input) {
        Arena *temporary = input->next;

#ifdef MISC_WINDOWS_HOST
        HANDLE handle = input->handle;
#endif

        size_t total = input->total + sizeof *input;
        MISC_BUILTIN_FREE(input, total);

#ifdef MISC_WINDOWS_HOST
        CloseHandle(handle);
#endif

        input = temporary;
    }
}

#ifndef MISC_USE_GLOBAL_ALLOCATOR

#ifndef MISC_ALLOC
#define MISC_ALLOC(size) calloc((size), 1)
#endif

#ifndef MISC_CALLOC
#define MISC_CALLOC(count, size) calloc((count), (size))
#endif

#ifndef MISC_REALLOC
#define MISC_REALLOC(ptr, old_size, new_size) realloc((ptr), (new_size))
#endif

#ifndef MISC_FREE
#define MISC_FREE(ptr) free((ptr))
#endif

#else

/*
IMPORTANT: This macro shall be called in every main function.
REQUIRED_MACRO: @MISC_USE_GLOBAL_ALLOCATOR
*/

#define ARENA_INIT()                                                         \
    do {                                                                     \
        _misc_global_allocator = arena_create(MISC_GLOBAL_ALLOCATOR_PAGING); \
        atexit(_misc_atexit_hook);                                             \
    } while (0)

// Never call this explicitly
#define ARENA_DEINIT()                      \
    do {                                    \
        arena_free(_misc_global_allocator); \
        _misc_global_allocator = NULL;      \
    } while (0)

static inline void _misc_atexit_hook(void)
{
    ARENA_DEINIT();
}

#define MISC_ALLOC(size) arena_alloc(_misc_global_allocator, (size))
#define MISC_CALLOC(count, size) arena_alloc(_misc_global_allocator, (count) * (size))
#define MISC_REALLOC(ptr, old_size, new_size) arena_realloc(_misc_global_allocator, (ptr), (old_size), (new_size))
#define MISC_FREE(ptr) (void)0

#endif
/* ===== ARENA SECTION ===== */

/* ===== VECTOR SECTION ===== */
#ifndef VECTOR_ALLOC_FREQ
#define VECTOR_ALLOC_FREQ ((8ULL) * MISC_WORD_SIZE)
#endif

#define vector_push(vector, TYPE, expr)                  \
    do {                                                 \
        TYPE _miscvarname_literal = (expr);              \
        vector_pushptr((vector), &_miscvarname_literal); \
    } while (0)

/* Struct Vector, is a dynamicaly allocated structure that behave similar like
array, it's items is stored in a contigous manner and cache-efficient. However,
the operation like appending the vector with new item or getting the specific
item at a certain location may be computed at runtime, because of how the struct
works.

Vector can work surprisingly well using the following requirements:
1. The vector must know the size for each item.

And the rest of such metadata is stored automatically within the vector like
capacity and length.

It might be slow compared to functionality defined in <libmis/list.h> because
task like appending/getting the item requiring the vector to copy each bytes
from the specified item.

But that cons is paid off because it can store almost anything you can imagine.
*/

typedef struct {
    uint8_t *items;
    size_t item_size;
    size_t length;
    size_t capacity;
} Vector;

static inline Vector vector_create_with(size_t init_capacity, size_t item_size)
{
    Vector new_vector = {
        .items = NULL,
        .item_size = item_size,
    };

    if (item_size == 0)
        return (Vector){0};
    else if (init_capacity == 0)
        return new_vector;

    new_vector.items = (uint8_t *) MISC_CALLOC(init_capacity, item_size);
    if (new_vector.items == 0)
        return new_vector;
    else
        new_vector.capacity = init_capacity;

    return new_vector;
}

static inline Vector vector_create(size_t item_size)
{
    /* Inherit */
    return vector_create_with(0, item_size);
}

static inline bool vector_resize(Vector *input, size_t into)
{
    uint8_t *temporary;

    if (input == NULL || input->capacity == into || input->item_size == 0)
        return false;

    temporary = (uint8_t *) MISC_REALLOC(input->items, input->item_size * input->capacity, input->item_size * into);
    if (temporary == NULL)
        return false;

    input->items = temporary;
    input->capacity = into;
    input->length = into < input->length ? into : input->length;

    return true;
}

static inline bool vector_reserve(Vector *input, size_t additional)
{
    return input != NULL ? vector_resize(input, input->capacity + additional)
                         : false;
}

static inline bool vector_make_fit(Vector *input)
{
    return vector_resize(input, input != NULL ? input->length : 0);
}

static inline size_t vector_remaining(Vector *input)
{
    if (input != NULL)
        return input->capacity - input->length;
    else
        return 0;
}

static inline void *vector_get(Vector *input, size_t index)
{
    if (input != NULL && index < input->length) {
        if (input->capacity > 0)
            return (input->items + (input->item_size * index));
    }
    return NULL;
}

static inline void *vector_get_pos(Vector *input, size_t position)
{
    if (input != NULL && position < input->capacity)
        return (input->items + (input->item_size * position));

    return NULL;
}

static inline void vector_pushptr(Vector *input, void *any_data)
{
    uint8_t *increment_ptr;

    if (input == NULL || any_data == NULL)
        return;

    if (input->capacity == 0) {
        if (!vector_resize(input, VECTOR_ALLOC_FREQ))
            return;

    } else if (vector_remaining(input) <= 1) {
        if (!vector_resize(input, input->capacity * 2))
            return;
    }

    increment_ptr = input->items + (input->item_size * input->length++);
    memcpy(increment_ptr, any_data, input->item_size);
}

static inline void vector_free(Vector *input)
{
    if (input != NULL) {
        if (input->items != NULL)
#ifndef MISC_USE_GLOBAL_ALLOCATOR
            MISC_FREE(input->items);
#else
            input->items = NULL;
#endif

        input->capacity = 0;
        input->length = 0;
    }
}
/* ===== VECTOR SECTION ===== */

/* ===== STRING SECTION ===== */
#ifndef CSTR
#define CSTR(string) ((char *) (string.buffer.items))
#endif

typedef struct {
    Vector buffer;
} String;

static inline String string_create_with(size_t init_capacity)
{
    return (String){vector_create_with(init_capacity, 1)};
}

static inline String string_create(void)
{
    /* Inherit */
    return string_create_with(0);
}

static inline void string_push(String *input, char character)
{
    /* Inherit */
    vector_push((Vector *) input, char, character);
    (void) input;
    (void) character;
}

static inline void string_pushstr(String *input, char *cstr)
{
    size_t length;

    if (cstr == NULL)
        return;

    length = strlen(cstr);
    if (length >= input->buffer.capacity) {
        if (!vector_reserve((Vector *) input, length + MISC_WORD_SIZE /* SAFE AREA */ ))
            return;
    }

    void *innerbuf = vector_get_pos((Vector *) input, input->buffer.length);
    memcpy(innerbuf, cstr, length);
    input->buffer.length += length;
}

static inline void string_free(String *input) {
    /* Inherit */
#ifndef MISC_USE_GLOBAL_ALLOCATOR
    vector_free((Vector *) input);
#else
    (void) input;
#endif
}

static inline String string_fromstr(char *cstr, size_t length)
{
    String string;

    if (cstr == NULL || length < 1)
        return string_create();

    string = string_create_with(length + 1);
    string_pushstr(&string, cstr);
    return string;
}
/* ===== STRING SECTION ===== */

/* ===== REFCOUNT SECTION ===== */
/* This is the implementation of a reference counting
originally https://github.com/jeraymond/refcount.git

By default, when you allocate something using malloc() or MISC_REALLOC(), the
object is gone by the time you call free(), but in reference counting, that's
not the case.

You see, if you use reference counting, you can increase it's lifetime count
(Strong), or make it dying (Weak). If the object's lifetime count is zero, the
object is released, however if the object count is still strong, the routine to
make it weak, which is refcount_weak(), is only decrease the object's lifetime
count by 1.

When you make the object strong/weak, there is a guard to make the count only
change in one thread, so that the count doesn't get messed up when you use the
object on a multi-threaded environment, and that guard is called a mutex.

Before you increase/decrease the count, the routine will check if its possible
to obtain a lock of a mutex. If it is, the mutex is locked and the current
thread on which this routine is being called will wait until the mutex is
unlocked. This way you don't encounter data race.

WARNING
Don't ever free() the reference counted object manually, if you want to release
it all, just use refcount_drop(). */

typedef struct {
    mtx_t mutex;
    void *data;
    size_t count;
} Refcount;

static inline bool refcount_lock(mtx_t *mutex)
{
    switch (mtx_trylock(mutex)) {
    case thrd_error:
       return false;

    case thrd_busy:
       if (mtx_unlock(mutex) != thrd_success)
           return false;

       if (mtx_trylock(mutex) != thrd_success)
           return false;
    }

    return true;
}

static inline void *refcount_get(void *object)
{
    const uint8_t *padded = (uint8_t *) object;
    return (void *) (padded - sizeof(Refcount));
}

static inline void *refcount_alloc(size_t size)
{
    Refcount *obj_template;
    uint8_t *slice_bytes;

    if ((obj_template = (Refcount *) calloc(1, sizeof *obj_template + size)) == NULL)
        return NULL;

    if (mtx_init(&obj_template->mutex, mtx_plain) != thrd_success) {
        free(obj_template);
        return NULL;
    }

    obj_template->count = 1;
    slice_bytes = (uint8_t *) obj_template;
    slice_bytes = slice_bytes + sizeof *obj_template;
    obj_template->data = slice_bytes;

    return (void *) slice_bytes;
}

static inline bool refcount_upgrade(void **object)
{
    Refcount *as_refcount;
    if (object == NULL || *object == NULL)
        return false;

    as_refcount = (Refcount *) refcount_get(*object);
    if (refcount_lock(&as_refcount->mutex)) {
        as_refcount->count++;
        mtx_unlock(&as_refcount->mutex);
        return true;
    }

    return false;
}

static inline bool refcount_degrade(void **object) {
    Refcount *as_refcount;
    bool mark_free;

    if (object == NULL || *object == NULL)
        return false;

    as_refcount = (Refcount *) refcount_get(*object);
    mark_free = false;

    if (refcount_lock(&as_refcount->mutex)) {
        if (as_refcount->count == 1)
            mark_free = true;
        else
            as_refcount->count--;

        mtx_unlock(&as_refcount->mutex);
    } else {
        return false;
    }

    if (mark_free) {
        mtx_destroy(&as_refcount->mutex);
        free(as_refcount);
        *object = NULL;
    }

    return true;
}

static inline void refcount_drop(void **object)
{
    while (refcount_degrade(object))
        ;
}

static inline size_t refcount_lifetime(void **object)
{
    Refcount *as_refcount;
    size_t object_lifetime;

    if (object == NULL || *object == NULL)
        return 0;

    as_refcount = (Refcount *) refcount_get(*object);
    if (refcount_lock(&as_refcount->mutex)) {
        object_lifetime = as_refcount->count;
        mtx_unlock(&as_refcount->mutex);
    } else {
        return 0;
    }

    return object_lifetime;
}
/* ===== REFCOUNT SECTION ===== */


/* ===== FILE SECTION ===== */
static inline char *read_from_stream(FILE *stream)
{
    int64_t offset_max = 0;

#ifdef MISC_WINDOWS_HOST
    _fseeki64(stream, 0, SEEK_END);
    offset_max = _ftelli64(stream);
    _fseeki64(stream, 0, SEEK_SET);
#elif defined(MISC_POSIX_HOST)
    fseeko(stream, 0, SEEK_END);
    offset_max = ftello(stream);
    fseeko(stream, 0, SEEK_SET);
#else
#error Platform is not supported
#endif

    if (offset_max > 0) {
        char *buffer = (char *) MISC_ALLOC(offset_max + 1);
        if (buffer == NULL)
            return NULL;

        if (fread(buffer, 1, offset_max, stream) > 0)
            return buffer;
        else
            MISC_FREE(buffer);
    }

    return NULL;
}

static inline char *read_from_path(const char *path)
{
    FILE *stream = fopen(path, "rb");
    if (stream != NULL) {
        char *buffer = read_from_stream(stream);
        fclose(stream);

        if (buffer != NULL)
            return buffer;
    }
    return NULL;
}
/* ===== FILE SECTION ===== */

/* ===== LINKED LIST SECTION ===== */

/* WARNING: DEPRECATED / COMPILER SPECIFIC FEATURE */
#if !defined(ADDRESS_OF) && !defined(__cplusplus) && !defined(MISC_WINDOWS_HOST)
#define ADDRESS_OF(T) (&(typeof(T)){T})
#endif

typedef struct RawForwardList RawForwardList;

struct RawForwardList {
    RawForwardList *next;
    void *item;
};

typedef struct {
    RawForwardList *head, *tail;
    size_t length, item_size;
} ForwardList;

static inline RawForwardList *r_forward_list_create(void *item, size_t size)
{
    if (item == NULL || size < 1)
        return NULL;

    uint8_t *buffer = (uint8_t *) MISC_ALLOC(sizeof(RawForwardList) + size);
    if (buffer == NULL)
        return NULL;

    RawForwardList *rfl = (RawForwardList *) (buffer + 0);
    rfl->item = (void *) (buffer + sizeof(RawForwardList));
    rfl->next = NULL;
    memcpy(rfl->item, item, size);

    return rfl;
}

static inline void r_forward_list_append(RawForwardList **input, void *item, size_t size)
{
    if (input == NULL || *input == NULL || item == NULL || size < 1)
        return;

    RawForwardList *newer = r_forward_list_create(item, size);
    if (newer != NULL) {
        RawForwardList *current = *input;
        current->next = newer;
        *input = newer;
    }
}

static inline RawForwardList *r_forward_list_get(RawForwardList *head, size_t position)
{
    if (head == NULL)
        return NULL;

    while (head->next != NULL && position--)
        head = head->next;

    return head;
}

static inline void *r_forward_list_get_item(RawForwardList *head, size_t position)
{
    RawForwardList *found_node = r_forward_list_get(head, position);
    if (found_node != NULL)
        return found_node->item;

    return NULL;
}

static inline void r_forward_list_prepend(RawForwardList **input, void *item, size_t size)
{
    if (input == NULL || *input == NULL || item == NULL || size < 1)
        return;

    RawForwardList *newer = r_forward_list_create(item, size);
    if (newer != NULL) {
        newer->next = *input;
        *input = newer;
    }
}

static inline void r_forward_list_free(RawForwardList *head)
{
#ifndef MISC_USE_GLOBAL_ALLOCATOR
    while (head != NULL) {
        RawForwardList *next = head->next;
        free(head);
        head = next;
    }
#else
    if (head != NULL)
        *head = (RawForwardList){0};
#endif
}

static inline void forward_list_append(ForwardList *input, void *item)
{
    if (input == NULL || input->item_size < 1 || item == NULL)
        return;

    if (input->head == NULL) {
        input->head = r_forward_list_create(item, input->item_size);
        if (input->head == NULL)
            return;

        input->tail = input->head;
        return;
    } else {
        r_forward_list_append(&input->tail, item, input->item_size);
    }

    input->length++;
}

static inline void forward_list_prepend(ForwardList *input, void *item)
{
    if (input == NULL || input->item_size < 1 || item == NULL)
        return;

    if (input->head == NULL) {
        input->head = r_forward_list_create(item, input->item_size);
        if (input->head == NULL)
            return;

        input->tail = input->head;
    } else {
        r_forward_list_prepend(&input->head, item, input->item_size);
    }

    input->length++;
}

static inline RawForwardList *forward_list_getentry(ForwardList *input, size_t index)
{
    if (input == NULL || input->length >= index)
        return NULL;

    return r_forward_list_get(input->head, index);
}

static inline void forward_list_free(ForwardList *input)
{
    if (input != NULL) {
        r_forward_list_free(input->head);
        input->head = NULL;
        input->tail = NULL;
        input->length = 0;
    }
}

#define LIST_FORWARD  (1)
#define LIST_BACKWARD (0)

typedef struct RawList RawList;

struct RawList {
    RawList *prev, *next;
    void *item;
};

static inline RawList *r_list_create(void *item, size_t size)
{
    if (item == NULL || size < 1)
        return NULL;

    RawList *newer = NULL;
    uint8_t *as_bytes = (uint8_t *) MISC_ALLOC(sizeof(RawList) + size);

    if (as_bytes != NULL) {
        newer = (RawList *) (as_bytes + 0);
        newer->prev = NULL;
        newer->next = NULL;
        newer->item = as_bytes + sizeof(RawList);
        memcpy(newer->item, item, size);
    }

    return newer;
}

static inline bool r_list_append(RawList **tail, void *item, size_t size) {
    if (tail == NULL || *tail == NULL || item == NULL || size < 1)
        return false;

    RawList *latter = r_list_create(item, size);
    if (latter != NULL) {
        RawList *current = *tail;
        current->next = latter;
        latter->prev = current;
        *tail = latter;

        return true;
    }

    return false;
}

static inline bool r_list_prepend(RawList **head, void *item, size_t size)
{
    if (head == NULL || *head == NULL || item == NULL || size < 1)
        return false;

    RawList *newer = r_list_create(item, size);
    if (newer != NULL) {
        RawList *current = *head;
        current->prev = newer;
        newer->next = current;
        *head = newer;

        return true;
    }

    return false;
}

static inline RawList *r_list_rewind(RawList **tail)
{
    if (tail == NULL || *tail == NULL)
        return NULL;

    while ((*tail)->prev != NULL)
        (*tail) = (*tail)->prev;

    return *tail;
}

static inline void r_list_free(RawList *tail)
{
#ifndef MISC_USE_GLOBAL_ALLOCATOR
    while (tail != NULL) {
        RawList *prev = tail->prev;
        MISC_FREE(tail);
        tail = prev;
    }
#else
    if (tail != NULL)
        *tail = (RawList){0};
#endif
}

static inline RawList *r_list_get(RawList **start_point, size_t index, int direction)
{
    RawList *visitor = NULL;
    size_t count = 0;

    if (start_point == NULL || *start_point == NULL)
        return NULL;

    visitor = *start_point;
    if (direction) {
        while (visitor->next != NULL && count < index)
            visitor = visitor->next;
    } else {
        while (visitor->prev != NULL && count < index)
            visitor = visitor->prev;
    }

    return visitor;
}

static inline void *r_list_get_item(RawList **start_point, size_t index, int direction)
{
    RawList *target = r_list_get(start_point, index, direction);
    if (target != NULL)
        return target->item;

    return NULL;
}

typedef struct {
    RawList *head, *tail;
    size_t item_size, length;
} List;

static inline void list_append(List *input, void *item)
{
    if (input == NULL || input->item_size < 1 || item == NULL)
        return;

    if (input->tail == NULL) {
        if ((input->tail = r_list_create(item, input->item_size)) == NULL)
            return;

        input->head = input->tail;
    } else {
        if (!r_list_append(&input->tail, item, input->item_size))
            return;
    }

    input->length++;
}

static inline void list_prepend(List *input, void *item) {
    if (input == NULL || item == NULL)
        return;

    if (input->tail == NULL) {
        if ((input->tail = r_list_create(item, input->item_size)) == NULL)
            return;

        input->head = input->tail;
    } else {
        if (!r_list_prepend(&input->tail, item, input->item_size))
            return;

        input->head = input->tail;
    }

    input->length++;
}

static inline void *list_popleft(List *input)
{
    if (input == NULL || input->length < 1)
        return NULL;

    void *item = input->head->item;
    RawList *next = input->head->next;

    input->head = next;
    input->length--;
    return item;
}

static inline void *list_popright(List *input)
{
    if (input == NULL || input->length < 1)
        return NULL;

    void *item = input->tail->item;
    RawList *prev = input->tail->prev;

    input->tail = prev;
    input->length--;
    return item;
}

static inline void list_free(List *input)
{
    if (input != NULL) {
        r_list_free(input->tail);
        input->head = NULL;
        input->tail = NULL;
        input->length = 0;
    }
}
/* ===== LINKED LIST SECTION ===== */

#if __STDC_VERSION__ >= 202000L

/* GENERIC TYPES */
#define VECTOR(T)        \
    struct {             \
        T *items;        \
        size_t capacity; \
        size_t length;   \
    }

#define VECTOR_FREE(v)            \
    do {                          \
        if ((v).items != NULL)    \
            MISC_FREE((v).items); \
        (v).items = NULL;         \
        (v).capacity = 0;         \
        (v).length = 0;           \
    } while (0);

#define VECTOR_PUSH(v, val)                                        \
    do {                                                           \
        if ((v).items == NULL || (v).capacity - (v).length <= 1) { \
            size_t tmpcapacity = (v).capacity;                     \
            VECTOR_RESERVE(v, VECTOR_ALLOC_FREQ);                  \
            if ((v).capacity == tmpcapacity)                       \
                break;                                             \
        }                                                          \
        (v).items[(v).length++] = (val);                           \
    } while (0)

#define VECTOR_RESIZE(v, into)                                                                                             \
    do {                                                                                                                   \
        if (!(into))                                                                                                       \
            VECTOR_FREE(v);                                                                                                \
        if ((v).items == NULL || !(v).capacity) {                                                                          \
            if (((v).items = MISC_ALLOC(VECTOR_ALLOC_FREQ * sizeof *(v).items)) == NULL)                                   \
                break;                                                                                                     \
            (v).capacity = VECTOR_ALLOC_FREQ;                                                                              \
            (v).length = 0;                                                                                                \
        } else {                                                                                                           \
            typeof((v).items) tmp = MISC_REALLOC((v).items, (v).capacity * sizeof *(v).items, (into) * sizeof *(v).items); \
            if (tmp == NULL)                                                                                               \
                break;                                                                                                     \
            (v).items = tmp;                                                                                               \
            (v).capacity = (into);                                                                                         \
        }                                                                                                                  \
    } while (0)

#define VECTOR_RESERVE(v, how_much)                  \
    do {                                             \
        if ((how_much) <= (v).capacity)              \
            break;                                   \
        VECTOR_RESIZE(v, (v).capacity + (how_much)); \
    } while (0)

#define VECTOR_MAKE_FIT(v) VECTOR_RESIZE(v, (v).length)
#define VECTOR_REMAIN(v) ((v).capacity - (v).length)

#define FWLIST(T)                                \
    struct {                                     \
        struct __private_inner_FWLIST {          \
            struct __private_inner_FWLIST *next; \
            T item;                              \
        } *head, *tail;                          \
        size_t length;                           \
    }

#define FWLIST_APPEND(fw, val)                             \
    do {                                                   \
        typeof((fw).head) latter = MISC_ALLOC(sizeof(fw)); \
        if (latter == NULL) break;                         \
        latter->next = NULL;                               \
        latter->item = (val);                              \
        if ((fw).head == NULL || !((fw).length)) {         \
            (fw).head = latter;                            \
            (fw).tail = latter;                            \
        } else {                                           \
            (fw).tail->next = latter;                      \
            (fw).tail = latter;                            \
        }                                                  \
        (fw).length++;                                     \
    } while (0)

#define FWLIST_PREPEND(fw, val)                            \
    do {                                                   \
        typeof((fw).head) latter = MISC_ALLOC(sizeof(fw)); \
        if (latter == NULL) break;                         \
        latter->next = NULL;                               \
        latter->item = (val);                              \
        if ((fw).head == NULL || !((fw).length)) {         \
            (fw).head = latter;                            \
            (fw).tail = latter;                            \
        } else {                                           \
            latter->next = (fw).head;                      \
            (fw).head = latter;                            \
        }                                                  \
        (fw).length++;                                     \
    } while (0)

#define FWLIST_POPLEFT(fw, stored_in)                           \
    do {                                                        \
        if ((fw).head == NULL || !((fw).length)) break;         \
        typeof((fw).head) popped = (fw).head;                   \
        (stored_in) = popped->item;                             \
        (fw).head = (fw).head->next;                            \
        (fw).length--;                                          \
        MISC_FREE(popped);                                      \
    } while (0)

#define FWLIST_FREE(fw)             \
    do {                            \
        typeof((fw).head->item) _i; \
        FWLIST_POPLEFT(fw, _i);     \
    } while ((fw).length != 0)

#endif

#ifdef MISC_SEE_EASTER_EGG
typedef VECTOR(FWLIST(int)) ComplicatedType;
/*

ComplicatedType {                             24 bytes
    .items = struct * {
        .head = __private_inner_FWLIST * {    8 bytes
            .next = __private_inner_FWLIST *, 8 bytes
            .item = int,                      4 bytes
        },
        .tail = __private_inner_FWLIST * {    8 bytes
            .next = __private_inner_FWLIST *, 8 bytes
            .item = int,                      4 bytes
        },
        .length = size_t,                     8 bytes
    },
    .capacity = size_t,                       8 bytes
    .length = size_t,                         8 bytes
};

*/

#endif

#if __STDC_VERSION__ >= 202000L

#ifndef MISC_FNV_PRIME
// FNV 64
#define MISC_FNV_PRIME ((uint64_t){1099511628211})
#endif

// FNV 32
#ifndef MISC_FNV_OCTET_BASIS
#define MISC_FNV_OCTET_BASIS ((uint64_t){2166136261})
#endif

#ifndef MISC_HTAB_DISTRESS_LIMIT
#define MISC_HTAB_DISTRESS_LIMIT (1ULL << 8ULL)
#endif

#define MISC_HTAB_MIN_LENGTH (8)
#define MISC_HTAB_COLLISION_INIT MISC_HTAB_DISTRESS_LIMIT

#ifndef MISC_ENTR_MIN_LENGTH
#define MISC_ENTR_MIN_LENGTH (64)
#endif

typedef struct {
    const char *key;
    size_t length;
} HashKey;

typedef struct {
    HashKey key;
    void *value;
} HashEntry;

typedef VECTOR(HashEntry) Bucket;
typedef VECTOR(Bucket) HashRoom;

typedef struct {
    HashRoom buckets;
    size_t coll_count, coll_cap;
} HashTable;

typedef uint64_t HashIndex;

static inline HashIndex misc_FNV1a(HashKey key)
{
    HashIndex basis = MISC_FNV_OCTET_BASIS;
    for (size_t i = 0; i < key.length; i++) {
        basis ^= key.key[i];
        basis ^= MISC_FNV_PRIME;
    }

    return basis;
}

static inline bool key_verify(HashKey lhs, HashKey rhs)
{
    size_t length_used;
    if (lhs.length <= rhs.length)
        length_used = lhs.length;
    else
        length_used = rhs.length;

    if (strncmp(lhs.key, rhs.key, length_used) == 0)
        return true;

    return false;
}

HashKey key_create_raw(const char *key, const size_t length)
{
    return (HashKey){key, length};
}

HashKey key_create(const char *key)
{
    return key_create_raw(key, strlen(key));
}

HashIndex key_to_hash(const char *term_key)
{
    return misc_FNV1a(key_create(term_key));
}

HashEntry entry_create_raw(const char *key,
                           const size_t key_length,
                           void *value)
{
    return (HashEntry){.key = key_create_raw(key, key_length), .value = value};
}

HashEntry entry_create(const char *key, void *value)
{
    return (HashEntry){key_create(key), value};
}

static inline int bucket_comparator(const void *lhs, const void *rhs)
{
    const HashKey *e_lhs = (const HashKey *) lhs;
    const HashEntry *e_rhs = (const HashEntry *) rhs;
    size_t length_used;

    if (e_lhs->length <= e_rhs->key.length)
        length_used = e_lhs->length;
    else
        length_used = e_rhs->key.length;

    return strncmp(e_lhs->key, e_rhs->key.key, length_used);
}

static inline void bucket_sort(Bucket *bucket)
{
    qsort((void *) bucket->items, bucket->length, sizeof(*(bucket->items)), bucket_comparator);
}

static inline HashEntry *bucket_find(Bucket *bucket, const HashKey signature)
{
    bucket_sort(bucket);
    return (HashEntry *) bsearch((const void *) &signature, (void *) bucket->items, bucket->length, sizeof(*(bucket->items)), bucket_comparator);
}

/* NOT INTENDED FOR PUBLIC USE */
static inline bool bucket_find_manual(Bucket *bucket,
                                      const HashKey signature,
                                      HashEntry **storage)
{
    for (size_t i = 0; i < bucket->length; i++) {
        if (key_verify(bucket->items[i].key, signature)) {
            *storage = bucket->items + i;
            return true;
        }
    }

    return false;
}

static inline HashTable table_create(void)
{
    HashTable table = {
        .coll_count = 0,
        .coll_cap = MISC_HTAB_COLLISION_INIT,
    };

    VECTOR_RESERVE(table.buckets, MISC_HTAB_DISTRESS_LIMIT);
    if (!table.buckets.capacity)
        return (HashTable){0};

    return table;
}

static inline bool table_insert(HashTable *htab, HashEntry entry)
{
    if (htab == NULL || entry.value == NULL || entry.key.key == NULL)
        return false;

    HashRoom *buckets = &htab->buckets;
    if (htab->coll_count == htab->coll_cap || VECTOR_REMAIN(*buckets) <= MISC_HTAB_MIN_LENGTH) {
       size_t save = buckets->capacity; 
       VECTOR_RESERVE(*buckets, MISC_HTAB_DISTRESS_LIMIT);

       if (save == buckets->capacity)
           return false;
    }

    HashIndex hash = misc_FNV1a(entry.key) % buckets->capacity;
    Bucket *bucket = &buckets->items[hash];

    if (bucket->length != 0) {
        VECTOR_PUSH(*bucket, entry);
        htab->coll_count++;
    } else {
        size_t tmpcap = bucket->capacity;
        VECTOR_RESERVE(*bucket, MISC_HTAB_COLLISION_INIT);

        if (tmpcap == bucket->capacity)
            return false;

        bucket->items[0] = entry;
        bucket->length++;
    }

    return true;
}

static inline bool table_delete(HashTable *htab, HashKey key)
{
    if (htab == NULL || htab->buckets.capacity < MISC_HTAB_DISTRESS_LIMIT || key.key == NULL || !key.length)
        return false;

    HashIndex hash = misc_FNV1a(key) % htab->buckets.capacity;
    Bucket *bucket = htab->buckets.items + hash;

    if (!bucket->length) // not found
        return false;

    HashEntry *incase_found = NULL;

    if (bucket->length <= MISC_ENTR_MIN_LENGTH) {
        bucket_sort(bucket);
        if (!bucket_find_manual(bucket, key, &incase_found))
            return false;
    } else {
        incase_found = bucket_find(bucket, key);
        if (incase_found == NULL)
            return false;
    }

    // Duh, don't free it, i'm saving memory here
    incase_found->value = NULL;
    memset(incase_found, 0, sizeof *incase_found);
    bucket->length--;
    return true;
}

static inline HashEntry *table_get(HashTable *htab, HashKey key)
{
    if (htab == NULL || key.key == NULL || !key.length)
        return (HashEntry *) NULL;

    HashIndex hash = misc_FNV1a(key) % htab->buckets.capacity;
    Bucket *bucket = htab->buckets.items + hash;

    if (!bucket->length)
        return NULL;
    else if (bucket->length == 1)
        return bucket->items;

    HashEntry *incase_found = NULL;
    if (bucket->length <= MISC_ENTR_MIN_LENGTH) {
        if (bucket_find_manual(bucket, key, &incase_found))
            return incase_found;

        return NULL;
    } else {
        return bucket_find(bucket, key);
    }
}

static inline void *table_getvalue(HashTable *htab, const HashKey key)
{
    HashEntry *entry = table_get(htab, key);
    if (entry == NULL || entry->value == NULL)
        return NULL;

    return entry->value;
}

static inline void table_free(HashTable *htab)
{
    if (htab != NULL) {
        for (size_t i = 0; i < htab->buckets.length; i++) {
            Bucket *bucket = &htab->buckets.items[i];
            VECTOR_FREE(*bucket);
        }

        VECTOR_FREE(htab->buckets);
        htab->coll_count = 0;
        htab->coll_cap = MISC_HTAB_COLLISION_INIT;
    }
}

#endif

/*
TODO:
1. Win32/64 Mapping function for arena (still use malloc-family function)
2. sbrk/brk option for arena

IMPORTANT:
3. automaticaly align the memory allocated by arena (i don't think mmap does that)
*/

#endif
