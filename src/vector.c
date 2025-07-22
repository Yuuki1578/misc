#include "./libmisc/vector.h"
#include <alloca.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Vector vector_with(const size_t init_capacity, const size_t item_size) {
  Vector v = {
      .items     = 0,
      .item_size = item_size,
      .capacity  = 0,
      .length    = 0,
  };

  if (item_size == 0)
    return (Vector){0};
  else if (init_capacity == 0)
    return v;

  v.items = (uintptr_t)calloc(init_capacity, item_size);
  if (v.items == 0)
    return v;
  else
    v.capacity = init_capacity;

  return v;
}

Vector vector_new(const size_t item_size) {
  // Inherit
  return vector_with(0, item_size);
}

bool vector_resize(Vector *v, const size_t into) {
  if (v == NULL || v->capacity == into || v->item_size == 0)
    return false;

  uintptr_t tmp = (uintptr_t)realloc((void *)v->items, v->item_size * into);
  if (tmp == 0)
    return false;

  v->items    = tmp;
  v->capacity = into;
  v->length   = into < v->length ? into : v->length;

  return true;
}

bool vector_make_fit(Vector *v) {
  return vector_resize(v, v != NULL ? v->length : 0);
}

size_t vector_remaining(const Vector *v) {
  if (v != NULL)
    return v->capacity - v->length;
  else
    return 0;
}

void *vector_at(const Vector *v, const size_t index) {
  if (v != NULL && index < v->length) {
    if (v->capacity > 0)
      return (void *)(v->items + (v->item_size * index));
  }
  return NULL;
}

void vector_push(Vector *v, const void *any) {
  if (v == NULL || any == NULL)
    return;

  if (v->capacity == 0) {
    if (!vector_resize(v, VECTOR_ALLOC_FREQ))
      return;

  } else if (vector_remaining(v) <= 1) {
    if (!vector_resize(v, v->capacity * 2))
      return;
  }

  uintptr_t increment = v->items + (v->item_size * v->length++);
  memcpy((void *)increment, any, v->item_size);
}

void vector_push_many(Vector *v, ...) {
  va_list va;
  void   *args;

  if (v == NULL)
    return;

  va_start(va, v);

  while ((args = va_arg(va, void *)) != NULL)
    vector_push(v, args);

  va_end(va);
}

void vector_free(Vector *v) {
  if (v != NULL) {
    if (v->items != 0)
      free((void *)v->items);
    v->items    = 0;
    v->capacity = 0;
    v->length   = 0;
  }
}
