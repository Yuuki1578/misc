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
