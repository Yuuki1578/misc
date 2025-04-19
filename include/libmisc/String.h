#ifndef MISC_STRING_H
#define MISC_STRING_H

#include <libmisc/Layout.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>

#define STRING_ALLOC_STEP 128ULL

#define STRING_STATUS_OK 0
#define STRING_STATUS_ERR -1

/*
 * Non-terminated and heap-allocated string type, String.
 * Use standard formatter if you want to print it to standard output.
 * */
typedef struct CedString {
  char* rawString;
  size_t length;
  Layout layout;
} String;

/*
 * A mutable unsigned 64-bit integer, use to set the byte
 * of every char* pushed onto the String.
 * */
extern size_t StringStepDefault;

/*
 * Initialize null String.
 * */
String StringNew(void);

/*
 * Allocate additional <count> bytes memory space to raw buffer.
 * Can be use to minimize the malloc() or realloc() call.
 * Return <count> on success, -1 on error.
 * */
ssize_t StringReserve(String* string, size_t count);

/*
 * Pushing a single character at the end of the raw buffer.
 * */
int StringPush(String* string, char ch);

/*
 * Pushing a char* at the end of the raw buffer.
 * */
ssize_t StringPushstr(String* string, char* cstr);

/*
 * Get a pointer to a single character at index <index>.
 * Return null if index >= String->length.
 * */
char* StringAt(String* string, size_t index);

/*
 * Truncate remaining unused bytes in the buffer.
 * The capacity is now equal to String->length.
 * */
int StringCrop(String* string);

/*
 * Deallocate the inner buffer.
 * */
void StringFree(String* string);

#endif
