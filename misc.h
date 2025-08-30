/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under MIT License. All right reserved.

*/

#ifndef MISC_H
#define MISC_H

#if defined(__unix__) || defined(__linux__)
#define _FILE_OFFSET_BITS 64
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

/* ===== ARENA SECTION ===== */
#define ARENA_PAGE (1ULL << 12ULL)
#define remain_of(arena) ((arena)->total - (arena)->offset)

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

static inline Arena* arena_create(size_t size)
{
    Arena* head = (Arena*)calloc(1, sizeof *head + size);
    if (!head)
        return NULL;

    head->next = NULL;
    head->total = size;
    head->offset = 0;
    head->data = ((uint8_t*)head) + sizeof *head;
    return head;
}

static inline Arena* __find_suitable_arena(Arena* base, size_t size, int* found)
{
    Arena *iter = base, *last_nonnull;
    while (iter) {
        if (remain_of(iter) >= size) {
            *found = 1;
            return iter;
        }

        last_nonnull = iter;
        iter = iter->next;
    }

    *found = 0;
    return last_nonnull;
}

static inline void* arena_alloc(Arena* arena, size_t size)
{
    Arena* suitable;
    int found;
    size_t size_required;

    if (!arena || !size)
        return NULL;

    suitable = __find_suitable_arena(arena, size, &found);
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

static inline void* arena_realloc(Arena* base, void* dst, size_t old_size, size_t new_size)
{
    void* result = arena_alloc(base, new_size);
    if (!result)
        return NULL;

    else if (!dst)
        return result;

    memcpy(result, dst, old_size > new_size ? new_size : old_size);
    return result;
}

static inline void arena_free(Arena* base)
{
    while (base) {
        Arena* tmp = base->next;
        free(base);
        base = tmp;
    }
}
/* ===== ARENA SECTION ===== */

/* ===== VECTOR SECTION ===== */
#ifndef VECTOR_ALLOC_FREQ
#define VECTOR_ALLOC_FREQ 8ULL
#endif

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
    uint8_t* items;
    size_t item_size;
    size_t length;
    size_t capacity;
} Vector;

static inline Vector vector_with(size_t init_capacity, size_t item_size)
{
    Vector vector = {
        .items = NULL,
        .item_size = item_size,
    };

    if (item_size == 0)
        return (Vector) { 0 };
    else if (init_capacity == 0)
        return vector;

    vector.items = (uint8_t*)calloc(init_capacity, item_size);
    if (vector.items == 0)
        return vector;
    else
        vector.capacity = init_capacity;

    return vector;
}

static inline Vector vector_new(size_t item_size)
{
    /* Inherit */
    return vector_with(0, item_size);
}

static inline bool vector_resize(Vector* v, size_t into)
{
    uint8_t* tmp;

    if (v == NULL || v->capacity == into || v->item_size == 0)
        return false;

    tmp = (uint8_t*)realloc(v->items, v->item_size * into);
    if (tmp == 0)
        return false;

    v->items = tmp;
    v->capacity = into;
    v->length = into < v->length ? into : v->length;

    return true;
}

static inline bool vector_make_fit(Vector* v)
{
    return vector_resize(v, v != NULL ? v->length : 0);
}

static inline size_t vector_remaining(Vector* v)
{
    if (v != NULL)
        return v->capacity - v->length;
    else
        return 0;
}

static inline void* vector_at(Vector* v, size_t index)
{
    if (v != NULL && index < v->length) {
        if (v->capacity > 0)
            return (v->items + (v->item_size * index));
    }
    return NULL;
}

static inline void vector_push(Vector* v, void* any)
{
    uint8_t* increment;

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
    memcpy(increment, any, v->item_size);
}

static inline void vector_push_many_fn(Vector* v, ...)
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

static inline void vector_free(Vector* v)
{
    if (v != NULL) {
        if (v->items != NULL)
            free(v->items);
        v->items = 0;
        v->capacity = 0;
        v->length = 0;
    }
}
/* ===== VECTOR SECTION ===== */

/* ===== STRING SECTION ===== */
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

static inline String string_with(size_t init_capacity)
{
    return (String) { vector_with(init_capacity, 1) };
}

static inline String string_new(void)
{
    /* Inherit */
    return string_with(0);
}

static inline void string_push(String* s, char ch)
{
    /* Inherit */
    vector_push((Vector*)s, &ch);
}

static inline void string_push_many_fn(String* s, ...)
{
    va_list va;
    va_start(va, s);
    vector_push_many((Vector*)s, va);
    va_end(va);
}

static inline void string_pushcstr(String* s, char* cstr)
{
    size_t len;

    if (cstr == NULL)
        return;

    len = strlen(cstr);
    while (len--)
        string_push(s, *cstr++);
}

static inline void string_pushcstr_many_fn(String* s, ...)
{
    va_list va;
    char* cstr;

    va_start(va, s);
    while ((cstr = va_arg(va, char*)) != NULL)
        string_pushcstr(s, cstr);

    va_end(va);
}

static inline void string_free(String* s)
{
    /* Inherit */
    vector_free((Vector*)s);
}

static inline String string_from(char* cstr, size_t len)
{
    String string;

    if (cstr == NULL || len < 1)
        return string_new();

    string = string_with(len + 1);
    string_pushcstr(&string, cstr);
    return string;
}
/* ===== STRING SECTION ===== */

/* ===== REFCOUNT SECTION ===== */
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

typedef struct {
    mtx_t mutex;
    void* raw_data;
    size_t count;
} Ref_Count;

static inline bool __refcount_lock(mtx_t* mutex)
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

static inline void* __get_refcount(void* object)
{
    const uint8_t* counter = (uint8_t*)object;
    return (void*)(counter - sizeof(Ref_Count));
}

static inline void* refcount_alloc(size_t size)
{
    Ref_Count* object_template;
    uint8_t* slice;

    if ((object_template = (Ref_Count*)calloc(sizeof *object_template + size, 1)) == NULL)
        return NULL;

    if (mtx_init(&object_template->mutex, mtx_plain) != thrd_success) {
        free(object_template);
        return NULL;
    }

    object_template->count = 1;
    slice = (uint8_t*)object_template;
    slice = slice + sizeof *object_template;
    object_template->raw_data = slice;

    return (void*)slice;
}

static inline bool refcount_strong(void** object)
{
    Ref_Count* counter;
    if (object == NULL || *object == NULL)
        return false;

    counter = (Ref_Count*)__get_refcount(*object);
    if (__refcount_lock(&counter->mutex)) {
        counter->count++;
        mtx_unlock(&counter->mutex);
        return true;
    }

    return false;
}

static inline bool refcount_weak(void** object)
{
    Ref_Count* counter;
    bool mark_as_free;

    if (object == NULL || *object == NULL)
        return false;

    counter = (Ref_Count*)__get_refcount(*object);
    mark_as_free = false;

    if (__refcount_lock(&counter->mutex)) {
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

static inline void refcount_drop(void** object)
{
    while (refcount_weak(object))
        ;
}

static inline size_t refcount_lifetime(void** object)
{
    Ref_Count* counter;
    size_t object_lifetime;

    if (object == NULL || *object == NULL)
        return 0;

    counter = (Ref_Count*)__get_refcount(*object);
    if (__refcount_lock(&counter->mutex)) {
        object_lifetime = counter->count;
        mtx_unlock(&counter->mutex);

    } else {
        return 0;
    }

    return object_lifetime;
}
/* ===== REFCOUNT SECTION ===== */

/* ===== LIST SECTION ===== */
#if __STDC_VERSION__ >= 201700L
#ifndef MISC_LIST_FREQ
#define MISC_LIST_FREQ (8ULL)
#endif

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
static inline char* __read_from_stream(FILE* file)
{
    int64_t offset_max = 0;

#if defined(_WIN32) || defined(_WIN64)
    _fseeki64(file, 0, SEEK_END);
    offset_max = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);
#elif defined(__unix__) || defined(__linux__)
    fseeko64(file, 0, SEEK_END);
    offset_max = ftello64(file);
    fseeko64(file, 0, SEEK_SET);
#endif

    if (offset_max > 0) {
        char* buffer = (char*)malloc(offset_max + 1);
        buffer != NULL ? fread(buffer, 1, offset_max, file) : 0;
        return buffer;
    }

    return NULL;
}

static inline char* file_read_all(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file != NULL) {
        char* buffer = __read_from_stream(file);
        fclose(file);

        if (buffer != NULL)
            return buffer;
    }
    return NULL;
}

static inline char* file_read_from(FILE* file)
{
    return file != NULL ? __read_from_stream(file) : NULL;
}
/* ===== FILE SECTION ===== */

/* ===== LINKED LIST SECTION ===== */
#if !defined(address_of) && !defined(__cplusplus)
#define address_of(T) (&(typeof(T)) { T })
#endif
#define for_link(list, var_name) for (Raw_List* var_name = list.list; var_name != NULL && var_name->item != NULL; var_name = var_name->next)

typedef struct Raw_Link Raw_Link;
struct Raw_Link {
    Raw_Link* next;
    void* item;
};

typedef struct {
    Raw_Link* list;
    size_t item_size, length;
    void (*on_free)(void* item, size_t item_size);
} Linked_List;

static inline Raw_Link* ll_get_last_node(Raw_Link* dst)
{
    if (dst == NULL)
        return NULL;

    while (dst->next != NULL)
        dst = dst->next;

    return dst;
}

static inline void ll_append(Linked_List* dst, void* item)
{
    Raw_Link** last = NULL;
    void** item_ref = NULL;
    uint8_t* room = NULL;

    if (dst == NULL || dst->item_size < 1 || item == NULL)
        return;

    if (dst->list == NULL)
        last = &dst->list;
    else {
        Raw_Link* last_nonnull = ll_get_last_node(dst->list);
        last = &last_nonnull->next;
    }

    room = (uint8_t*)calloc(1, sizeof(Raw_Link) + dst->item_size);
    if (room == NULL)
        return;

    *last = (Raw_Link*)(room + 0);
    (*last)->next = NULL;
    item_ref = &(*last)->item;
    *item_ref = (void*)(room + sizeof(Raw_Link));

    memcpy(*item_ref, item, dst->item_size);
    dst->length++;
}

static inline void ll_prepend(Linked_List* dst, void* item)
{
    Raw_Link* front = NULL;
    uint8_t* room = NULL;

    if (dst == NULL || dst->item_size == 0 || item == NULL)
        return;

    room = (uint8_t*)calloc(1, sizeof(Raw_Link) + dst->item_size);
    if (room != NULL) {
        front = (Raw_Link*)(room + 0);
        front->item = (void*)(room + sizeof(Raw_Link));
        front->next = dst->list;

        memcpy(front->item, item, dst->item_size);
        dst->list = front;
        dst->length++;
    }
}

static inline void ll_free(Linked_List* dst)
{
    Raw_Link* tmp = NULL;

    if (dst == NULL)
        return;
    else
        tmp = dst->list;

    while (tmp != NULL) {
        Raw_Link* next = tmp->next;
        if (dst->on_free != NULL)
            dst->on_free(tmp->item, dst->item_size);

        free(tmp);
        tmp = next;
    }

    dst->list = NULL;
}

static inline Raw_Link* ll_get_node(Linked_List* dst, size_t index)
{

    Raw_Link* current = NULL;

    if (dst == NULL || dst->length <= index)
        return NULL;
    else
        current = dst->list;

    for (size_t i = 0;; i++, current = current->next) {
        if (i == index)
            return current;
    }
}

static inline void* ll_get_item(Linked_List* dst, size_t index)
{
    Raw_Link* list = ll_get_node(dst, index);

    if (list != NULL)
        return list->item;
    else
        return NULL;
}

#define DLINK_FORWARD (1)
#define DLINK_BACKWARD (0)

typedef struct Raw_Double_Link Raw_Double_Link;
struct Raw_Double_Link {
    Raw_Double_Link *prev, *next;
    void* item;
};

static inline Raw_Double_Link* rdl_new(void* inhabitan, size_t size)
{
    if (inhabitan == NULL || size < 1)
        return NULL;

    Raw_Double_Link* chain = NULL;
    uint8_t* room = (uint8_t*)calloc(1, sizeof(Raw_Double_Link) + size);

    if (room != NULL) {
        chain = (Raw_Double_Link*)(room + 0);
        chain->prev = NULL;
        chain->next = NULL;
        chain->item = room + sizeof(Raw_Double_Link);
        memcpy(chain->item, inhabitan, size);
    }

    return chain;
}

static inline bool rdl_append(Raw_Double_Link** tail, void* inhabitan, size_t size)
{
    if (tail == NULL || *tail == NULL || inhabitan == NULL || size < 1)
        return false;

    Raw_Double_Link* latter = rdl_new(inhabitan, size);
    if (latter != NULL) {
        Raw_Double_Link* now = *tail;
        now->next = latter;
        latter->prev = now;
        *tail = latter;

        return true;
    }

    return false;
}

static inline bool rdl_prepend(Raw_Double_Link** tail, void* inhabitan, size_t size)
{
    if (tail == NULL || *tail == NULL || inhabitan == NULL || size < 1)
        return false;

    Raw_Double_Link* new_head = rdl_new(inhabitan, size);
    if (new_head != NULL) {
        Raw_Double_Link* now = *tail;
        now->prev = new_head;
        new_head->next = now;
        *tail = new_head;

        return true;
    }

    return false;
}

static inline Raw_Double_Link* rdl_rewind(Raw_Double_Link** tail)
{
    if (tail == NULL || *tail == NULL)
        return NULL;

    while ((*tail)->prev != NULL)
        (*tail) = (*tail)->prev;

    return *tail;
}

static inline void rdl_free(Raw_Double_Link* tail)
{
    while (tail != NULL) {
        Raw_Double_Link* prev = tail->prev;
        free(tail);
        tail = prev;
    }
}

static inline Raw_Double_Link* rdl_get_node(Raw_Double_Link** node, size_t index, int direction)
{
    Raw_Double_Link* iter = NULL;
    size_t count = 0;

    if (node == NULL || *node == NULL)
        return NULL;

    iter = *node;
    if (direction) {
        while (iter->next != NULL && count < index)
            iter = iter->next;
    } else {
        while (iter->prev != NULL && count < index)
            iter = iter->prev;
    }

    return iter;
}

static inline void* rdl_get_item(Raw_Double_Link** node, size_t index, int direction)
{
    Raw_Double_Link* node_target = rdl_get_node(node, index, direction);
    if (node_target != NULL)
        return node_target->item;

    return NULL;
}

typedef struct {
    Raw_Double_Link *head, *tail;
    size_t item_size, length;
} Double_Link;

static inline void dl_append(Double_Link* dst, void* item)
{
    if (dst == NULL || dst->item_size < 1 || item == NULL)
        return;

    if (dst->tail == NULL) {
        if ((dst->tail = rdl_new(item, dst->item_size)) == NULL)
            return;

        dst->head = dst->tail;
    } else {
        if (!rdl_append(&dst->tail, item, dst->item_size))
            return;
    }

    dst->length++;
}

static inline void dl_prepend(Double_Link* dst, void* item)
{
    if (dst == NULL || item == NULL)
        return;

    if (dst->tail == NULL) {
        if ((dst->tail = rdl_new(item, dst->item_size)) == NULL)
            return;

        dst->head = dst->tail;
    } else {
        if (!rdl_prepend(&dst->tail, item, dst->item_size))
            return;

        dst->head = dst->tail;
    }

    dst->length++;
}

static inline void dl_free(Double_Link* dst)
{
    if (dst != NULL)
        rdl_free(dst->tail);
}
/* ===== LINKED LIST SECTION ===== */

#endif
