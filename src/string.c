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
