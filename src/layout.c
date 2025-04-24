// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file layout.c
// @brief memory layout structure

#include <libmisc/layout.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

Layout layout_new(uint16_t size, size_t dflen) {
  if (size == 0) {
    size = sizeof(char);
  }

  return (Layout){
      .size = size,
      .needed = size * dflen,
      .status = LAYOUT_NULL_PTR,
  };
}

void layout_add(Layout* self, size_t count) {
  if (self == nullptr || count == 0) {
    return;
  }

  if (self->needed == SIZE_MAX) {
    return;
  }

  if (self->size == 0) {
    self->size = sizeof(char);
  }

  self->needed += self->size * count;
}

void layout_min(Layout* self, size_t count) {
  if (self == nullptr || count == 0) {
    return;
  }

  size_t currentTotal = self->size * count;

  if (self->needed < currentTotal || self->needed == 0) {
    return;
  }

  self->needed -= currentTotal;
}

void* layout_alloc(Layout* self) {
  void* alloc;
  size_t needed;

  if (self == nullptr) {
    return malloc(0);
  }

  if (self->size == 0) {
    self->size = sizeof(char);
  }

  if (self->needed == 0) {
    alloc = malloc(0);

    if (alloc == nullptr) {
      self->status = LAYOUT_NULL_PTR;
    } else {
      self->status = LAYOUT_UNIQUE_PTR;
    }

    return alloc;
  }

  needed = self->needed;
  alloc = malloc(needed);

  if (alloc == nullptr) {
    self->status = LAYOUT_NULL_PTR;
  } else {
    self->status = LAYOUT_NON_NULL;
  }

  return alloc;
}

void* layout_realloc(Layout* self, void* target) {
  size_t needed;
  void* from_realloc;

  if (self == nullptr || target == nullptr) {
    return layout_alloc(nullptr);
  }

  needed = self->needed;
  from_realloc = realloc(target, needed);

  if (from_realloc == nullptr) {
    self->status = LAYOUT_NULL_PTR;
  } else {
    self->status = LAYOUT_NON_NULL;
  }

  return from_realloc;
}

void layout_dealloc(Layout* self, void* target) {
  if (self == nullptr) {
    if (target != nullptr) {
      free(target);
    }

    return;
  }

  if (self->status != LAYOUT_NULL_PTR && target != NULL) {
    free(target);
  }

  // invalidate the pointer
  target = nullptr;
  self->needed = 0;
  self->status = LAYOUT_NULL_PTR;
}
