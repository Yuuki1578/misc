#ifndef MISC_VECTOR_H
#define MISC_VECTOR_H

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

Vector vector_with(size_t init_capacity, size_t item_size);
Vector vector_new(size_t item_size);
bool vector_resize(Vector* v, size_t into);
bool vector_make_fit(Vector* v);
size_t vector_remaining(Vector* v);
void* vector_at(Vector* v, size_t index);
void vector_push(Vector* v, void* any);
void vector_free(Vector* v);
void vector_push_many_fn(Vector* v, ...); // use the macro instead!

#endif
