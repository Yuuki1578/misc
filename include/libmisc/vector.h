#ifndef LIBMISC_VECTOR
#define LIBMISC_VECTOR

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define VECTOR_ALLOC_FREQ 8ULL

typedef struct {
    uintptr_t items;
    size_t item_size;
    size_t length;
    size_t capacity;
} Vector;

Vector vector_with(size_t init_capacity, size_t item_size);
Vector vector_new(size_t item_size);
bool vector_add_capacity(Vector *v, size_t how_much);
bool vector_resize(Vector *v, size_t into);
size_t vector_remaining(Vector *v);
void *vector_at(Vector *v, size_t index);
void vector_make_fit(Vector *v);
void vector_push(Vector *v, void *any);
void vector_push_many(Vector *v, ...) __attribute__((sentinel));
void vector_free(Vector *v);

#endif
