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

#include <libmisc/Vector.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Vector VectorWith(const size_t initCapacity, const size_t itemSize) {
  Vector vector;
  vector.items    = 0;
  vector.itemSize = itemSize;
  vector.capacity = 0;
  vector.length   = 0;

  if (itemSize == 0)
    return (Vector){0};
  else if (initCapacity == 0)
    return vector;

  vector.items = (uintptr_t)calloc(initCapacity, itemSize);
  if (vector.items == 0)
    return vector;
  else
    vector.capacity = initCapacity;

  return vector;
}

Vector VectorNew(const size_t itemSize) {
  /* Inherit */
  return VectorWith(0, itemSize);
}

bool VectorResize(Vector *v, const size_t into) {
  uintptr_t tmp;

  if (v == NULL || v->capacity == into || v->itemSize == 0)
    return false;

  tmp = (uintptr_t)realloc((void *)v->items, v->itemSize * into);
  if (tmp == 0)
    return false;

  v->items    = tmp;
  v->capacity = into;
  v->length   = into < v->length ? into : v->length;

  return true;
}

bool VectorMakeFit(Vector *v) {
  return VectorResize(v, v != NULL ? v->length : 0);
}

size_t VectorRemaining(const Vector *v) {
  if (v != NULL)
    return v->capacity - v->length;
  else
    return 0;
}

void *VectorAt(const Vector *v, const size_t index) {
  if (v != NULL && index < v->length) {
    if (v->capacity > 0)
      return (void *)(v->items + (v->itemSize * index));
  }
  return NULL;
}

void VectorPush(Vector *v, const void *any) {
  uintptr_t increment;

  if (v == NULL || any == NULL)
    return;

  if (v->capacity == 0) {
    if (!VectorResize(v, VECTOR_ALLOC_FREQ))
      return;

  } else if (VectorRemaining(v) <= 1) {
    if (!VectorResize(v, v->capacity * 2))
      return;
  }

  increment = v->items + (v->itemSize * v->length++);
  memcpy((void *)increment, any, v->itemSize);
}

void VectorPushMany(Vector *v, ...) {
  va_list va;
  void   *args;

  if (v == NULL)
    return;

  va_start(va, v);

  while ((args = va_arg(va, void *)) != NULL)
    VectorPush(v, args);

  va_end(va);
}

void VectorFree(Vector *v) {
  if (v != NULL) {
    if (v->items != 0)
      free((void *)v->items);
    v->items    = 0;
    v->capacity = 0;
    v->length   = 0;
  }
}
