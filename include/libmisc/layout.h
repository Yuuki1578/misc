#ifndef CED_LAYOUT_H
#define CED_LAYOUT_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef enum : int8_t {
  LAYOUT_UNIQUE_PTR = -1,
  LAYOUT_NULL_PTR = 0,
  LAYOUT_NON_NULL = 1,
} Status;

#define MEMORY_ALLOC_SUCCESS 0
#define MEMORY_ALLOC_FAILURE -1

/*
 * simple memory layout, providing basic features such as:
 * 1. the size of the type
 * 2. the memory capacity
 * 3. the pointer length
 * 4. status of the current block of memory, which can be:
 *   1. UNIQUE_PTR == -1
 *   2. NULL_PTR == 0
 *   3. NON_NULL == 1
 * */

typedef struct CedMemoryLayout {
  Status status;
  uint16_t size;
  size_t capacity;
  size_t requested;
} Layout;

/*
 * create a new layout, if the size == 0, it's set to sizeof(char) by default
 * */
Layout LayoutNew(uint16_t size, size_t defaultLength);

/*
 * adding necessary element to the Layout, incrementing the length.
 * */
void LayoutAdd(Layout *layout, size_t count);

/*
 * removing count bytes element from the back.
 * This function does not reallocating
 * */
void LayoutMin(Layout *layout, size_t count);

/*
 * allocate memory from the heap, based on Layout->capacity using malloc()
 * */
void *LayoutAlloc(Layout *layout);

/*
 * reallocating the memory, based on Layout->capacity
 * */
void *LayoutRealloc(Layout *layout, void *target);

/*
 * deallocating the memory, returning the memory back to the operating system
 * and set the Layout capacity and length to zero, but keep the type size
 * untouched
 * */
void LayoutDealloc(Layout *layout, void *target);

#endif
