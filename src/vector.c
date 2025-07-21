#include <libmisc/vector.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

Vector vector_with(size_t init_capacity,
                   size_t item_size)
{
    Vector v = {
        .items = 0,
        .item_size = item_size,
        .capacity = 0,
        .length = 0,
    };

    if (item_size == 0)
        return (Vector){0};
    else if (init_capacity == 0)
        return v;

    v.items = (uintptr_t) calloc(init_capacity, item_size);
    if (v.items == 0)
        return v;
    else
        v.capacity = init_capacity;

    return v;
}

Vector vector_new(size_t item_size)
{
    return vector_with(0, item_size);
}

bool vector_add_capacity(Vector *v,
                         size_t how_much)
{
    if (v == NULL || v->item_size == 0)
        return false;

    if (v->capacity >= how_much || how_much == 0)
        return false;

    if (v->capacity == 0) {
        v->items = (uintptr_t) calloc(how_much, v->item_size);
        if (v->items == 0)
            return false;
        v->capacity = how_much;
    } else {
        uintptr_t tmp = (uintptr_t) realloc((void *) v->items, v->item_size * (v->capacity + how_much));
        if (tmp == 0)
            return false;
        v->items = tmp;
        v->capacity += how_much;
    }
    return true;
}

bool vector_resize(Vector *v, size_t into)
{
    if (v == NULL || into == v->capacity)
        return false;
    if (v->capacity == 0)
        return vector_add_capacity(v, into);

    uintptr_t tmp = (uintptr_t) realloc((void *) v->items, v->item_size * into);
    if (tmp == 0)
        return false;
    if (into < v->length)
        v->length = into;

    v->capacity = into;
    return true;
}

size_t vector_remaining(Vector *v)
{
    if (v != NULL)
        return v->capacity - v->length;
    else
        return 0;
}

void *vector_at(Vector *v, size_t index)
{
    if (v != NULL && index < v->length) {
        if (v->capacity > 0)
            return (void *)(v->items + (v->item_size * index));
    }
    return NULL;
}

void vector_make_fit(Vector *v)
{
    vector_resize(v, v->length);
}

void vector_push(Vector *v, void *any)
{
    if (v == NULL || any == NULL)
        return;
    
    if (vector_remaining(v) <= 1) {
        size_t needed = 0;
        if (v->capacity == 0)
            needed = VECTOR_ALLOC_FREQ;
        else
            needed *= 2;
        
        if (!vector_add_capacity(v, needed))
            return;
    } else {
        uintptr_t target = v->items + (v->item_size * v->length++);
        memcpy((void *) target, any, v->item_size);
    }
}

void vector_push_many(Vector *v, ...)
{
    va_list va;
    void *args;

    if (v == NULL)
        return;

    va_start(va, v);

    while ((args = va_arg(va, void *)) != NULL)
        vector_push(v, args);

    va_end(va);
}

void vector_free(Vector *v)
{
    if (v != NULL) {
        if (v->items != 0)
            free((void *) v->items);
        v->items = 0;
        v->capacity = 0;
        v->length = 0;
    }
}
