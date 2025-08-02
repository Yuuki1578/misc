/* The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

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
