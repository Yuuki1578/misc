// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file layout.c
// @brief memory layout structure

#include <libmisc/layout.h>
#include <stdint.h>
#include <stdlib.h>

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

void layout_add(Layout* layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  if (layout->needed == SIZE_MAX) {
    return;
  }

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  layout->needed += layout->size * count;
}

void layout_min(Layout* layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  size_t currentTotal = layout->size * count;

  if (layout->needed < currentTotal || layout->needed == 0) {
    return;
  }

  layout->needed -= currentTotal;
}

void* layout_alloc(Layout* layout) {
  void* alloc;
  size_t needed;

  if (layout == nullptr) {
    return malloc(0);
  }

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  if (layout->needed == 0) {
    alloc = malloc(0);

    if (alloc == nullptr) {
      layout->status = LAYOUT_NULL_PTR;
    } else {
      layout->status = LAYOUT_UNIQUE_PTR;
    }

    return alloc;
  }

  needed = layout->needed;
  alloc = malloc(needed);

  if (alloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  return alloc;
}

void* layout_realloc(Layout* layout, void* target) {
  size_t needed;
  void* from_realloc;

  if (layout == nullptr || target == nullptr) {
    return layout_alloc(nullptr);
  }

  needed = layout->needed;
  from_realloc = realloc(target, needed);

  if (from_realloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  return from_realloc;
}

void layout_dealloc(Layout* layout, void* target) {
  if (layout == nullptr) {
    if (target != nullptr) {
      free(target);
    }

    return;
  }

  if (layout->status != LAYOUT_NULL_PTR && target != NULL) {
    free(target);
  }

  // invalidate the pointer
  target = nullptr;
  layout->needed = 0;
  layout->status = LAYOUT_NULL_PTR;
}
