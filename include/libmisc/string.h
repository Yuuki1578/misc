// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file string.h
// @brief dynamically allocated string type

#ifndef MISC_STRING_H
#define MISC_STRING_H

#include <libmisc/layout.h>
#include <limits.h>
#include <stddef.h>

#ifdef _WIN32
#include <stdint.h>
typedef int64_t ssize_t;
#else
#include <sys/types.h>
#endif

#define STRING_ALLOC_STEP 128ULL

#define STRING_STATUS_OK 0
#define STRING_STATUS_ERR -1

/*
 * Non-terminated and heap-allocated string type, String.
 * Use standard formatter if you want to print it to standard output.
 * */
typedef struct DynString {
  char* rawptr;
  size_t length;
  Layout layout;
} String;

/*
 * A mutable unsigned 64-bit integer, use to set the byte
 * of every char* pushed onto the String.
 * */
extern size_t STRING_STEP_DFL;

/*
 * Initialize null String.
 * */
extern String string_new(void);

/*
 * Allocate additional <count> bytes memory space to raw buffer.
 * Can be use to minimize the malloc() or realloc() call.
 * Return <count> on success, -1 on error.
 * */
extern ssize_t string_reserve(String* string, size_t count);

/*
 * Pushing a single character at the end of the raw buffer.
 * */
extern int string_push(String* string, char ch);

/*
 * Pushing a char* at the end of the raw buffer.
 * */
extern ssize_t string_pushstr(String* string, char* cstr);

/*
 * Get a pointer to a single character at index <index>.
 * Return null if index >= String->length.
 * */
extern char* string_at(String* string, size_t index);

/*
 * Truncate remaining unused bytes in the buffer.
 * The capacity is now equal to String->length.
 * */
extern int string_crop(String* string);

/*
 * Deallocate the inner buffer.
 * */
extern void string_free(String* string);

#endif
