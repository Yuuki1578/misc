#ifndef CED_STRING_H
#define CED_STRING_H

#include <libmisc/layout.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>

#define STRING_ALLOC_STEP 128ULL

#define STRING_STATUS_OK 0
#define STRING_STATUS_ERR -1

/*
 * non-terminated + heap-allocated string type: string_t
 * use standard formatter if you want to print it to standard output
 * the functionality of this type is same as vector of uint8_t
 * */
typedef struct CedString {
  char *rawString;
  size_t length;
  Layout layout;
} String;

extern size_t StringStepDefault;

/* METADATA */
#define STR(string) ((string).raw_str == nullptr ? "" : (string).raw_str)
#define CAP(string) ((string).layout.cap)
#define LEN(string) ((string).len)

/*
 * initialize null string
 * */
String StringNew(void);

/*
 * allocate additional <count> bytes memory space to raw buffer
 * can be use to minimalize the malloc() or realloc() call
 * */
ssize_t StringReserve(String *string, size_t count);

/*
 * pushing a single character at the end of the raw buffer
 * */
int StringPush(String *string, char ch);

/*
 * pushing a C-string at the end of the raw buffer
 * */
ssize_t StringPushstr(String *string, char *cstr);

/*
 * return the address of a single character at index <index>
 * return null if index >= string_t->len
 * */
char *StringAt(String *string, size_t index);

/*
 * truncate remaining unused bytes in the buffer
 * the capacity is now string_t->len * layout_t->t_size
 * */
void StringCrop(String *string);

/*
 * deallocate the inner buffer, freeing it's memory
 * */
void StringFree(String *string);

#endif
