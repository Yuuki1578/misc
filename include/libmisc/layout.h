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
 * 1. the size of the type.
 * 2. the memory needed.
 * 3. status of the current block of memory, which can be:
 *   • LAYOUT_UNIQUE_PTR == -1.
 *   • LAYOUT_NULL_PTR == 0.
 *   • LAYOUT_NON_NULL == 1.
 * */

typedef struct CedMemoryLayout {
  Status status;
  uint16_t size;
  size_t needed;
} Layout;

/*
 * Create a new Layout, if the size == 0, set to sizeof(char) by default.
 * */
Layout LayoutNew(uint16_t size, size_t defaultLength);

/*
 * Adding <count> element to the Layout, incrementing the length.
 * */
void LayoutAdd(Layout *layout, size_t count);

/*
 * Removing <count> bytes element.
 * This function does not reallocating.
 * */
void LayoutMin(Layout *layout, size_t count);

/*
 * Allocate memory from the heap, based on Layout->needed using malloc()
 * */
void *LayoutAlloc(Layout *layout);

/*
 * Reallocating the memory, based on Layout->needed using realloc()
 * */
void *LayoutRealloc(Layout *layout, void *target);

/*
 * Deallocating the memory.
 * The Layout->size is remain untouched for later usage.
 * */
void LayoutDealloc(Layout *layout, void *target);

#endif
