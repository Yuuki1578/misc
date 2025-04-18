#include <libmisc/layout.h>
#include <stdint.h>
#include <stdlib.h>

Layout LayoutNew(uint16_t size, size_t defaultLength) {
  if (size == 0) {
    size = sizeof(char);
  }

  return (Layout){
      .size = size,
      .capacity = 0,
      .requested = defaultLength,
      .status = LAYOUT_NULL_PTR,
  };
}

void LayoutAdd(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  layout->requested += count;
}

void LayoutMin(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  if (layout->requested < count) {
    return;
  }

  layout->requested -= count;
}

void *LayoutAlloc(Layout *layout) {
  void *fromAlloc;
  size_t currentCapacity;

  if (layout == nullptr) {
    return malloc(0);
  }

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  if (layout->capacity == 0) {
    fromAlloc = malloc(0);

    if (fromAlloc == nullptr) {
      layout->status = LAYOUT_NULL_PTR;
    } else {
      layout->status = LAYOUT_UNIQUE_PTR;
    }

    return fromAlloc;
  }

  currentCapacity = layout->requested * layout->size;
  fromAlloc = malloc(currentCapacity);

  if (fromAlloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  layout->capacity = currentCapacity;
  return fromAlloc;
}

void *LayoutRealloc(Layout *layout, void *target) {
  size_t currentCapacity;
  void *fromAlloc;

  if (layout == nullptr || target == nullptr) {
    return LayoutAlloc(nullptr);
  }

  currentCapacity = layout->requested * layout->size;
  fromAlloc = realloc(target, currentCapacity);

  if (fromAlloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
    layout->capacity = currentCapacity;
  }

  return target;
}

void LayoutDealloc(Layout *layout, void *target) {
  if (layout == nullptr) {
    if (target != nullptr) {
      free(target);
    }

    return;
  }

  if (layout->status != LAYOUT_NULL_PTR) {
    free(target);
  }

  // invalidate the pointer
  target = nullptr;
  layout->capacity = 0;
  layout->requested = 0;
  layout->status = LAYOUT_NULL_PTR;
}
