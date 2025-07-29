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

#include "../include/libmisc/string.h"
#include <limits.h>
#include <stdarg.h>
#include <string.h>

String string_with(size_t init_capacity)
{
    return (String) { vector_with(init_capacity, 1) };
}

String string_new(void)
{
    /* Inherit */
    return string_with(0);
}

String string_from(char* cstr, size_t len)
{
    String string;

    if (cstr == NULL || len < 1)
        return string_new();

    string = string_with(len + 1);
    string_pushcstr(&string, cstr);
    return string;
}

void string_push(String* s, char ch)
{
    /* Inherit */
    vector_push((Vector*)s, &ch);
}

void string_push_many_fn(String* s, ...)
{
    va_list va;
    va_start(va, s);
    vector_push_many((Vector*)s, va);
    va_end(va);
}

void string_pushcstr(String* s, char* cstr)
{
    register size_t len;

    if (cstr == NULL)
        return;

    len = strlen(cstr);
    while (len--)
        string_push(s, *cstr++);
}

void string_pushcstr_many_fn(String* s, ...)
{
    va_list va;
    char* cstr;

    va_start(va, s);
    while ((cstr = va_arg(va, char*)) != NULL)
        string_pushcstr(s, cstr);

    va_end(va);
}

void string_free(String* s)
{
    /* Inherit */
    vector_free((Vector*)s);
}
