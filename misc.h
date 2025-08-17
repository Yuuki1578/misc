/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under MIT License. All right reserved.

*/

#ifndef MISC_H
#define MISC_H

/* ===== ARENA SECTION ===== */
#ifndef MISC_ARENA
#define MISC_ARENA

#include <stddef.h>
#include <stdint.h>

#define ARENA_PAGE (1ULL << 12ULL)
#define remainof(arena) ((arena)->total - (arena)->offset)

/* Arena types, a single linked list that point to the next allocator.
For a better portability, we using uint8_t* instead of raw void* on a pointer
arithmetic context.

You create the arena using arena_create() that accept 1 arguments, the initial size.

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
arena_alloc() will create a new allocator, pointed by @next with the
following rules:
1. If the requested size is larger than the size of the past allocator, the size
   of the new allocator would be (size * 2).
2. If it's not, the size will be (past_allocator->total * 2).

The arena, roughly, would look like this:

[4096] -> [8192] -> [16384] -> [32768]

The chunk returned by arena_alloc() or arena_realloc() may be NULL, so you must
check it before using it.

Now for the best part is that we only need to free our arena's once and we're
done with it. Other neat thing is that you can use the arena and pass it around
to a bunch of function, so that you know that those section of your program need
to allocate some memory.

NOTE:
Since the arena is just a linked list, it search for a suitable arena to perform
allocation by lineary check if the arena is match the requirement (the size).
So it would be better to create a big chunk of arena. The default one is ARENA_PAGE
or 4096 bytes (same as in my system, see the getpagesize(2)), which is enough for
everyone nowadays. */

typedef struct Arena Arena;

struct Arena {
    Arena* next;
    size_t total, offset;
    uint8_t* data;
};

#include <stdlib.h>
#include <string.h>

inline Arena* arena_create(size_t size)
{
    Arena* head = calloc(1, sizeof *head + size);
    if (!head)
        return NULL;

    head->next = NULL;
    head->total = size;
    head->offset = 0;
    head->data = ((uint8_t*)head) + sizeof *head;
    return head;
}

inline Arena* find_suitable_arena(Arena* base, size_t size, int* found)
{
    Arena *iter = base, *last_nonnull;
    while (iter) {
        if (remainof(iter) >= size) {
            *found = 1;
            return iter;
        }

        last_nonnull = iter;
        iter = iter->next;
    }

    *found = 0;
    return last_nonnull;
}

inline void* arena_alloc(Arena* arena, size_t size)
{
    Arena* suitable;
    int found;
    size_t size_required;

    if (!arena || !size)
        return NULL;

    suitable = find_suitable_arena(arena, size, &found);
    size_required = size > suitable->total ? size * 2 : suitable->total * 2;

    if (!found) {
        suitable->next = arena_create(size_required);
        if (!suitable->next)
            return NULL;

        suitable = suitable->next;
    }

    void* result = suitable->data + suitable->offset;
    suitable->offset += size;
    return result;
}

inline void* arena_realloc(Arena* base, void* dst, size_t old_size, size_t new_size)
{
    void* result = arena_alloc(base, new_size);
    if (!result)
        return NULL;

    else if (!dst)
        return result;

    memcpy(result, dst, old_size > new_size ? new_size : old_size);
    return result;
}

inline void arena_free(Arena* base)
{
    while (base) {
        Arena* tmp = base->next;
        free(base);
        base = tmp;
    }
}

#endif
/* ===== ARENA SECTION ===== */

/* ===== VECTOR SECTION ===== */
#ifndef MISC_VECTOR
#define MISC_VECTOR

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VECTOR_ALLOC_FREQ 8ULL
#define vector_push_many(vector, ...) \
    vector_push_many_fn(vector, __VA_ARGS__, ((void*)0))

/* Struct Vector, is a dynamicaly allocated structure that behave similar like array,
it's items is stored in a contigous manner and cache-efficient. However, the operation
like appending the vector with new item or getting the specific item at a certain
location may be computed at runtime, because of how the struct works.

Vector can work surprisingly well using the following requirements:
1. The vector must know the size for each item.

And the rest of such metadata is stored automatically within the vector like capacity
and length.

It might be slow compared to functionality defined in <libmis/list.h> because task like
appending/getting the item requiring the vector to copy each bytes from the specified item.

But that cons is paid off because it can store almost anything you can imagine. */

typedef struct {
    uintptr_t items;
    size_t item_size;
    size_t length;
    size_t capacity;
} Vector;

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

inline Vector vector_with(size_t init_capacity, size_t item_size)
{
    Vector vector = {
        .items = 0,
        .item_size = item_size,
        .capacity = 0,
        .length = 0,
    };

    if (item_size == 0)
        return (Vector) { 0 };
    else if (init_capacity == 0)
        return vector;

    vector.items = (uintptr_t)calloc(init_capacity, item_size);
    if (vector.items == 0)
        return vector;
    else
        vector.capacity = init_capacity;

    return vector;
}

inline Vector vector_new(size_t item_size)
{
    /* Inherit */
    return vector_with(0, item_size);
}

inline bool vector_resize(Vector* v, size_t into)
{
    uintptr_t tmp;

    if (v == NULL || v->capacity == into || v->item_size == 0)
        return false;

    tmp = (uintptr_t)realloc((void*)v->items, v->item_size * into);
    if (tmp == 0)
        return false;

    v->items = tmp;
    v->capacity = into;
    v->length = into < v->length ? into : v->length;

    return true;
}

inline bool vector_make_fit(Vector* v)
{
    return vector_resize(v, v != NULL ? v->length : 0);
}

inline size_t vector_remaining(Vector* v)
{
    if (v != NULL)
        return v->capacity - v->length;
    else
        return 0;
}

inline void* vector_at(Vector* v, size_t index)
{
    if (v != NULL && index < v->length) {
        if (v->capacity > 0)
            return (void*)(v->items + (v->item_size * index));
    }
    return NULL;
}

inline void vector_push(Vector* v, void* any)
{
    uintptr_t increment;

    if (v == NULL || any == NULL)
        return;

    if (v->capacity == 0) {
        if (!vector_resize(v, VECTOR_ALLOC_FREQ))
            return;

    } else if (vector_remaining(v) <= 1) {
        if (!vector_resize(v, v->capacity * 2))
            return;
    }

    increment = v->items + (v->item_size * v->length++);
    memcpy((void*)increment, any, v->item_size);
}

inline void vector_push_many_fn(Vector* v, ...)
{
    va_list va;
    void* args;

    if (v == NULL)
        return;

    va_start(va, v);

    while ((args = va_arg(va, void*)) != NULL)
        vector_push(v, args);

    va_end(va);
}

inline void vector_free(Vector* v)
{
    if (v != NULL) {
        if (v->items != 0)
            free((void*)v->items);
        v->items = 0;
        v->capacity = 0;
        v->length = 0;
    }
}

#endif
/* ===== VECTOR SECTION ===== */

/* ===== STRING SECTION ===== */
#ifndef MISC_STRING
#define MISC_STRING

#ifndef CSTR
#define CSTR(string) ((char*)(string.vector.items))
#endif

#define string_push_many(string, ...) \
    string_push_many_fn(string, __VA_ARGS__, '\0')

#define string_pushcstr_many(string, ...) \
    string_pushcstr_many_fn(string, __VA_ARGS__, ((void*)0))

typedef struct {
    Vector vector;
} String;

#include <limits.h>
#include <stdarg.h>
#include <string.h>

inline String string_with(size_t init_capacity)
{
    return (String) { vector_with(init_capacity, 1) };
}

inline String string_new(void)
{
    /* Inherit */
    return string_with(0);
}

inline void string_push(String* s, char ch)
{
    /* Inherit */
    vector_push((Vector*)s, &ch);
}

inline void string_push_many_fn(String* s, ...)
{
    va_list va;
    va_start(va, s);
    vector_push_many((Vector*)s, va);
    va_end(va);
}

inline void string_pushcstr(String* s, char* cstr)
{
    register size_t len;

    if (cstr == NULL)
        return;

    len = strlen(cstr);
    while (len--)
        string_push(s, *cstr++);
}

inline void string_pushcstr_many_fn(String* s, ...)
{
    va_list va;
    char* cstr;

    va_start(va, s);
    while ((cstr = va_arg(va, char*)) != NULL)
        string_pushcstr(s, cstr);

    va_end(va);
}

inline void string_free(String* s)
{
    /* Inherit */
    vector_free((Vector*)s);
}

inline String string_from(char* cstr, size_t len)
{
    String string;

    if (cstr == NULL || len < 1)
        return string_new();

    string = string_with(len + 1);
    string_pushcstr(&string, cstr);
    return string;
}

#endif
/* ===== STRING SECTION ===== */

/* ===== REFCOUNT SECTION ===== */
#ifndef MISC_REFCOUNT
#define MISC_REFCOUNT

#include <stdbool.h>
#include <stddef.h>
#include <threads.h>

/* This is the implementation of a reference counting
originally https://github.com/jeraymond/refcount.git

By default, when you allocate something using malloc() or realloc(), the
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

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    mtx_t mutex;
    void* raw_data;
    size_t count;
} RefCount;

inline bool refcount_lock(mtx_t* mutex)
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

inline void* get_refcount(void* object)
{
    const uint8_t* counter = object;
    return (void*)(counter - sizeof(RefCount));
}

inline void* refcount_alloc(size_t size)
{
    RefCount* object_template;
    uint8_t* slice;

    if ((object_template = calloc(sizeof *object_template + size, 1)) == NULL)
        return NULL;

    if (mtx_init(&object_template->mutex, mtx_plain) != thrd_success) {
        free(object_template);
        return NULL;
    }

    object_template->count = 1;
    slice = (void*)object_template;
    slice = slice + sizeof *object_template;
    object_template->raw_data = slice;

    return (void*)slice;
}

inline bool refcount_strong(void** object)
{
    RefCount* counter;
    if (object == NULL || *object == NULL)
        return false;

    counter = get_refcount(*object);
    if (refcount_lock(&counter->mutex)) {
        counter->count++;
        mtx_unlock(&counter->mutex);
        return true;
    }

    return false;
}

inline bool refcount_weak(void** object)
{
    RefCount* counter;
    bool mark_as_free;

    if (object == NULL || *object == NULL)
        return false;

    counter = get_refcount(*object);
    mark_as_free = false;

    if (refcount_lock(&counter->mutex)) {
        if (counter->count == 1)
            mark_as_free = true;
        else
            counter->count--;

        mtx_unlock(&counter->mutex);
    } else {
        return false;
    }

    if (mark_as_free) {
        mtx_destroy(&counter->mutex);
        free(counter);
        *object = NULL;
    }

    return true;
}

inline void refcount_drop(void** object)
{
    while (refcount_weak(object))
        ;
}

inline size_t refcount_lifetime(void** object)
{
    RefCount* counter;
    size_t object_lifetime;

    if (object == NULL || *object == NULL)
        return 0;

    counter = get_refcount(*object);
    if (refcount_lock(&counter->mutex)) {
        object_lifetime = counter->count;
        mtx_unlock(&counter->mutex);

    } else {
        return 0;
    }

    return object_lifetime;
}

#endif
/* ===== REFCOUNT SECTION ===== */

/* ===== LIST SECTION ===== */
#ifndef MISC_LIST
#define MISC_LIST

#include <stddef.h>
#include <stdlib.h>

#define MISC_LIST_FREQ (8ULL)

/* NOTE:
The macros below are works on all struct implementing the field listed here:
1. <Type> *items.
2. size_t capacity.
3. size_t length.

Where <Type> are some concrete types. */

/* Anonymous struct, a generic-like type for C.
Examples:

    int main(void)
    {
        List(int) some_integer = {0};
        List(List(char*)) some_list_of_list_of_string = {0};

        list_append(some_integer, 10);
    }

*/
#define List(T)          \
    struct {             \
        T* items;        \
        size_t capacity; \
        size_t length;   \
    }

/* Resizing the list up and/or down. */
#define list_resize(list, size)                                                                                     \
    do {                                                                                                            \
        if ((list).capacity < 1 || (list).items == NULL) {                                                          \
            (list).items = (typeof((list).items))calloc((size), sizeof(typeof(*(list).items)));                     \
            if ((list).items == NULL)                                                                               \
                break;                                                                                              \
        } else {                                                                                                    \
            typeof((list).items) tmp = (typeof((list).items))realloc((list).items, (size) * sizeof(*(list).items)); \
            if ((list).items == NULL)                                                                               \
                break;                                                                                              \
            (list).items = tmp;                                                                                     \
        }                                                                                                           \
        if ((size) < (list).length)                                                                                 \
            (list).length = size;                                                                                   \
        (list).capacity = size;                                                                                     \
    } while (0)

/* Make the list fitting to it's length. */
#define list_make_fit(list)                                        \
    do {                                                           \
        if ((list).length < 1 || (list).capacity == (list).length) \
            break;                                                 \
        list_resize(list, (list).length);                          \
    } while (0)

/* Append an item to the list, increasing it's length. */
#define list_append(list, item)                                        \
    do {                                                               \
        if ((list).capacity < 1) {                                     \
            list_resize(list, MISC_LIST_FREQ);                         \
        } else if ((list).capacity - (list).length <= 1) {             \
            list_resize(list, (list).capacity * 2);                    \
        }                                                              \
        (list).items[(list).length++] = (typeof(*(list).items))(item); \
    } while (0)

/* Freeing the list, truncating it's capacity to zero. */
#define list_free(list)                                  \
    do {                                                 \
        if ((list).capacity > 0 || (list).items != NULL) \
            free((list).items);                          \
        (list).capacity = 0;                             \
        (list).length = 0;                               \
    } while (0)

#endif
/* ===== LIST SECTION ===== */

/* ===== FILE SECTION ===== */
#ifndef MISC_FILE
#define MISC_FILE

#include <stdio.h>
#include <stdlib.h>

inline char* read_from_stream(FILE* file)
{
    size_t bufsiz = BUFSIZ, readed = 0;
    char* buffer;

    if (file == NULL)
        return NULL;

    if ((buffer = calloc(1, bufsiz + 1)) == NULL)
        return NULL;

    while ((readed += fread(buffer + readed, 1, bufsiz, file)) > 0) {
        char* temporary = realloc(buffer, bufsiz * 2);
        if (temporary == NULL)
            return buffer;

        buffer = temporary;
        bufsiz *= 2;
    }

    buffer = realloc(buffer, readed + 1);
    if (buffer == NULL)
        return NULL;

    buffer[readed] = '\0';
    return buffer;
}

inline char* file_readall(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file != NULL) {
        char* buffer = read_from_stream(file);
        fclose(file);

        if (buffer != NULL)
            return buffer;
    }
    return NULL;
}

inline char* file_readfrom(FILE* file)
{
    return read_from_stream(file);
}

#endif
/* ===== FILE SECTION ===== */

#endif
