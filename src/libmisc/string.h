#ifndef MISC_STRING_H
#define MISC_STRING_H

#include "./vector.h"

typedef struct {
  Vector vector;
} String;

String string_with(const size_t init_capacity);
String string_new(void);
void   string_push(String *s, const char ch);
void   string_push_many(String *s, ...) /* NULL TERMINATED */;
void   string_push_cstr(String *s, const char *cstr);
void   string_push_cstr_many(String *s, ...) __attribute__((sentinel));
void   string_free(String *s);

#endif
