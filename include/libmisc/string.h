#ifndef MISC_STRING_H
#define MISC_STRING_H

#include "vector.h"

#ifndef CSTR
#define CSTR(string) ((char*)(string.vector.items))
#endif

#define string_push_many(string, ...) \
    string_push_many_fn(string, __VA_ARGS__, '\0')

#define string_pushcstr_many(string, ...) \
    string_pushcstr_many_fn(string, __VA_ARGS__, ((void*)0))

typedef struct {
    Vector vector;
} String;

String string_with(size_t init_capacity);
String string_new(void);
String string_from(char* cstr, size_t len);
void string_push(String* s, char ch);
void string_pushcstr(String* s, char* cstr);
void string_push_many_fn(String* s, ...); // use the macro instead!
void string_pushcstr_many_fn(String* s, ...); // use the macro instead!
void string_free(String* s);

#endif
