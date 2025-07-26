/* The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

#ifndef MISC_STRING_H
#define MISC_STRING_H

#include "vector.h"

#ifndef CSTR
#    define CSTR(string) ((char *)(string.vector.items))
#endif

#define string_push_many(string, ...)                                          \
    string_push_many_fn(string, __VA_ARGS__, '\0')

#define string_pushcstr_many(string, ...)                                      \
    string_pushcstr_many_fn(string, __VA_ARGS__, ((void *)0))

typedef struct {
    Vector vector;
} String;

String string_with(size_t init_capacity);
String string_new(void);
String string_from(char *cstr, size_t len);
void   string_push(String *s, char ch);
void   string_pushcstr(String *s, char *cstr);
void   string_push_many_fn(String *s, ...);     // use the macro instead!
void   string_pushcstr_many_fn(String *s, ...); // use the macro instead!
void   string_free(String *s);

#endif
