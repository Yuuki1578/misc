// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file layout.h
// @brief memory layout structure

#ifndef MISC_LAYOUT_H
#define MISC_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

typedef enum : int8_t {
  LAYOUT_UNIQUE_PTR = -1,
  LAYOUT_NULL_PTR = 0,
  LAYOUT_NON_NULL = 1,
} Status;

constexpr auto MEMORY_ALLOC_SUCCESS = 0;
constexpr auto MEMORY_ALLOC_FAILURE = -1;

/*
 * simple memory layout, providing basic features such as:
 * 1. the size of the type.
 * 2. the memory needed.
 * 3. status of the current block of memory, which can be:
 *   • LAYOUT_UNIQUE_PTR == -1.
 *   • LAYOUT_NULL_PTR == 0.
 *   • LAYOUT_NON_NULL == 1.
 * */

typedef struct {
  Status status;
  uint16_t size;
  size_t needed;
} Layout;

/*
 * Create a new Layout, if the size == 0, set to sizeof(char) by default.
 * */
extern Layout layout_new(uint16_t size, size_t defaultLength);

/*
 * Adding <count> element to the Layout, incrementing the length.
 * */
extern void layout_add(Layout* layout, size_t count);

/*
 * Removing <count> bytes element.
 * This function does not reallocating.
 * */
extern void layout_min(Layout* layout, size_t count);

/*
 * Allocate memory from the heap, based on Layout->needed using malloc()
 * */
extern void* layout_alloc(Layout* layout);

/*
 * Reallocating the memory, based on Layout->needed using realloc()
 * */
extern void* layout_realloc(Layout* layout, void* target);

/*
 * Deallocating the memory.
 * The Layout->size is remain untouched for later usage.
 * */
extern void layout_dealloc(Layout* layout, void* target);

#endif
