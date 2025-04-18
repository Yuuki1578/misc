#ifndef CED_MEMORY_H
#define CED_MEMORY_H

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

// default limit is set to 3GiB (reasonable)
#define CED_ALLOC_LIMIT     ((((3ULL * 1024ULL) * 1024ULL) * 1024ULL))

// unspecified allocation
#define CED_ALLOC_UNSPEC    0ULL

typedef enum : int8_t {
    UNIQUE_PTR  = -1,
    NULL_PTR    = 0,
    NON_NULL    = 1,
} status_t;

#define ALLOC_SUCCESS       0
#define ALLOC_FAILURE       -1

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
typedef struct {
    uint16_t    t_size;
    size_t      cap;
    size_t      len;
    status_t    status;
} layout_t;

/*
 * create a new layout, if the t_size == 0, it's set to sizeof(char) by default
 * */
layout_t layout_new(uint16_t t_size, size_t default_len);

/*
 * adding necessary element to the Layout, incrementing the length and capacity
 * */
void layout_add(layout_t *layout, size_t count);

/*
 * removing count bytes element from the back
 * this function does not reallocating
 * */
void layout_min(layout_t *layout, size_t count);

/*
 * allocate memory from the heap, based on Layout->cap using malloc()
 * flag is used to tell the function that, if the successful memory
 * recieved from the allocator, should be set to zero or not
 * */
void *layout_alloc(layout_t *layout);

/*
 * reallocating the memory, based on Layout->capacity
 * */
void *layout_realloc(layout_t *layout, void *dst);

/*
 * deallocating the memory, returning the memory back to the operating system
 * and set the Layout capacity and length to zero, but keep the type size
 * untouched
 * */
void layout_dealloc(layout_t *layout, void *dst);

#endif
