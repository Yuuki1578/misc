#ifndef CED_MEMORY_H
#define CED_MEMORY_H

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
  uint16_t t_size;
  size_t cap;
  size_t len;
} Layout;

/*
 * create a new layout, if the t_size == 0, it's set to sizeof(char) by default
 * */
Layout layout_new(uint16_t t_size, size_t default_len);

/*
 * adding necessary element to the Layout, incrementing the length and capacity
 * */
void layout_add(Layout *layout, size_t count);

/*
 * removing count bytes element from the back
 * this function does not reallocating
 * */
void layout_min(Layout *layout, size_t count);

/*
 * allocate memory from the heap, based on Layout->cap using malloc()
 * flag is used to tell the function that, if the successful memory
 * recieved from the allocator, should be set to zero or not
 * */
void *layout_alloc(Layout *layout);

/*
 * reallocating the memory, based on Layout->capacity
 * */
void *layout_realloc(Layout *layout, void *dst);

/*
 * deallocating the memory, returning the memory back to the operating system
 * and set the Layout capacity and length to zero, but keep the type size
 * untouched
 * */
void layout_dealloc(Layout *layout, void *dst);

#endif
