#ifndef LIBMISC_VECTOR
#define LIBMISC_VECTOR

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VECTOR_ALLOC_FREQ 8ULL

typedef struct {
  uintptr_t items;
  size_t    item_size;
  size_t    length;
  size_t    capacity;
} Vector;

Vector vector_with(const size_t init_capacity, const size_t item_size);
Vector vector_new(const size_t item_size);
bool   vector_resize(Vector *v, const size_t into);
bool   vector_make_fit(Vector *v);
size_t vector_remaining(const Vector *v);
void  *vector_at(const Vector *v, const size_t index);
void   vector_push(Vector *v, const void *any);
void   vector_push_many(Vector *v, ...) __attribute__((sentinel));
void   vector_free(Vector *v);

#endif
