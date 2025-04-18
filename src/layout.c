#include <libmisc/layout.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

Layout LayoutNew(uint16_t size, size_t defaultLength) {
  if (size == 0) {
    size = sizeof(char);
  }

  return (Layout){
      .size = size,
      .needed = size * defaultLength,
      .status = LAYOUT_NULL_PTR,
  };
}

void LayoutAdd(Layout *layout, size_t count) {
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

void LayoutMin(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  size_t currentTotal = layout->size * count;

  if (layout->needed < currentTotal || layout->needed == 0) {
    return;
  }

  layout->needed -= currentTotal;
}

void *LayoutAlloc(Layout *layout) {
  void *fromAlloc;
  size_t needed;

  if (layout == nullptr) {
    return malloc(0);
  }

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  if (layout->needed == 0) {
    fromAlloc = malloc(0);

    if (fromAlloc == nullptr) {
      layout->status = LAYOUT_NULL_PTR;
    } else {
      layout->status = LAYOUT_UNIQUE_PTR;
    }

    return fromAlloc;
  }

  needed = layout->needed;
  fromAlloc = malloc(needed);

  if (fromAlloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  return fromAlloc;
}

void *LayoutRealloc(Layout *layout, void *target) {
  size_t needed;
  void *fromRealloc;

  if (layout == nullptr || target == nullptr) {
    return LayoutAlloc(nullptr);
  }

  needed = layout->needed;
  fromRealloc = realloc(target, needed);

  if (fromRealloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  return fromRealloc;
}

void LayoutDealloc(Layout *layout, void *target) {
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
