#include <libmisc/vector.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Vector VectorWith(size_t len, size_t item_size) {
  Vector v = {
      .items     = NULL,
      .item_size = item_size,
      .len       = len,
      .cap       = len,
  };

  if (item_size == 0)
    return (Vector){0};

  if (len != 0) {
    v.items = calloc(len, item_size);
    if (v.items == NULL)
      return (Vector){0};
  }

  return v;
}

Vector VectorNew(size_t item_size) {
  // Zeroed
  return VectorWith(0, item_size);
}

bool VectorReserve(Vector *v, size_t how_much) {
  if (v == NULL || how_much == 0)
    return false;

  if (v->item_size == 0 || v->cap >= how_much)
    return false;

  if (v->items == NULL || v->cap == 0) {
    v->items = calloc(how_much, v->item_size * how_much);
    if (v->items == NULL)
      return false;
  } else {
    void *tmp = realloc(v->items, (v->item_size * (v->cap + how_much)));
    if (tmp == NULL)
      return false;

    v->items = tmp;
  }

  v->cap += how_much;
  return true;
}

size_t VectorRemaining(Vector *v) {
  if (v == NULL)
    return 0;

  return v->cap - v->len;
}

void *VectorAt(Vector *v, size_t index) {
  if (v == NULL || index >= v->len)
    return NULL;

  if (v->items == NULL)
    return NULL;

  return v->items + (v->item_size * index);
}

bool VectorPush(Vector *v, void *any) {
  void *tmp;

  if (v == NULL || any == NULL)
    return false;

  if (v->item_size == 0)
    return false;

  if (v->items == NULL) {
    if ((v->items = calloc(VECTOR_EACH_HARDCODED, v->item_size)) == NULL)
      return false;

    v->cap += VECTOR_EACH_HARDCODED;
  } else if (VectorRemaining(v) == 0) {
    tmp = realloc(v->items, (v->cap + VECTOR_EACH_HARDCODED) * v->item_size);
    if (tmp == NULL)
      return false;

    v->items = tmp;
    v->cap += VECTOR_EACH_HARDCODED;
  }

  memcpy(v->items + (v->item_size * v->len), any, v->item_size);
  v->len++;
  return true;
}

void VectorFree(Vector *v) {
  if (v == NULL)
    return;

  if (v->items != NULL) {
    free(v->items);
  }

  v->items = NULL;
  v->len   = 0;
  v->cap   = 0;
}

bool VectorAlign(Vector *v, size_t alignment) {
  uintptr_t aligned;

  if (v == NULL || v->items == NULL || v->cap == 0)
    return false;

  if (alignment == 0)
    return false;

  if ((alignment & (alignment - 1)) != 0)
    return false;

  aligned  = (uintptr_t)v->items;
  aligned  = (aligned + (alignment - 1)) & ~(alignment - 1);
  v->items = (void *)aligned;
  return true;
}
