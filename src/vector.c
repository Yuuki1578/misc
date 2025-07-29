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

#include "../include/libmisc/vector.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Vector vector_with(size_t init_capacity, size_t item_size)
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

Vector vector_new(size_t item_size)
{
    /* Inherit */
    return vector_with(0, item_size);
}

bool vector_resize(Vector* v, size_t into)
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

bool vector_make_fit(Vector* v)
{
    return vector_resize(v, v != NULL ? v->length : 0);
}

size_t vector_remaining(Vector* v)
{
    if (v != NULL)
        return v->capacity - v->length;
    else
        return 0;
}

void* vector_at(Vector* v, size_t index)
{
    if (v != NULL && index < v->length) {
        if (v->capacity > 0)
            return (void*)(v->items + (v->item_size * index));
    }
    return NULL;
}

void vector_push(Vector* v, void* any)
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

void vector_push_many_fn(Vector* v, ...)
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

void vector_free(Vector* v)
{
    if (v != NULL) {
        if (v->items != 0)
            free((void*)v->items);
        v->items = 0;
        v->capacity = 0;
        v->length = 0;
    }
}
