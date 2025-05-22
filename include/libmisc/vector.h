/*
 * April 2025, [https://github.com/Yuuki1578/misc.git]
 * This is a part of the libmisc library.
 * Any damage caused by this software is not my responsibility at all.

 * @file vector.h
 * @brief header only dynamic vector
 *
 * One thing to be concerned is that, if you use the macro provided in this header file
 * Your program might have slightly bigger binary size, because the macro is expanded and
 * hardcoded into binary itself, maybe the behavior is same as inline function
 *
 * */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * The size for each allocation can be modified through this macro
 * example:
 *
 * #define VECTOR_STEP 1024     // define first
 * #include <libmisc/vector.h>  // include later
 * ...
 *
 * */
#ifndef VECTOR_STEP
#define VECTOR_STEP 8
#endif

/*
 * Vector initialization value
 * Put simply, i don't want the user had a garbage value
 *
 * */
#define VECTOR_NEW          \
    {                       \
        .elems  = nullptr,  \
        .cap    = 0,        \
        .len    = 0,        \
    }

/*
 * Vector types
 * You might be dissapointed, but the type of this is just
 * an anonymous struct with generic type <Type>
 *
 * The user named the anonymous struct
 * You can obtain the type of this struct by using typeof() operator
 * example:
 *
 * vector(int) ints = VECTOR_NEW;
 * typeof(ints) foo = ints;
 * ...
 *
 * or:
 *
 * typedef typeof(vector(int)) VectorInt;
 * ...
 *
 * Also, you can use static array on this macro, because it'll break
 * at preprocessor stage
 *
 * you can use:
 *
 * vector(struct dirent*) dirs;
 *
 * instead of:
 *
 * vector(struct dirent[restrict 8]) dirs;
 * ...
 *
 * */
#define vector(Type)    \
    struct {            \
        Type    *elems; \
        size_t  cap;    \
        size_t  len;    \
    }

/*
 * Reserve some capacity for later use
 * The behavior is as follow:
 *
 * 1. If the vector is empty, then it's same as allocating <count> bytes into vector
 * 2. If the vector isn't empty, then the vector is reallocated to string.cap + <count>
 *
 * */
#define vector_reserve(vector, count)                                                                               \
    do {                                                                                                            \
        if (vector.cap == 0) {                                                                                      \
            vector.elems = calloc(count, sizeof(*vector.elems));                                                    \
            if (vector.elems == nullptr)                                                                            \
                break;                                                                                              \
        }                                                                                                           \
        else {                                                                                                      \
            typeof(vector.elems) backup = realloc(vector.elems, (vector.cap + count) * sizeof(*vector.elems));      \
            if (backup == nullptr)                                                                                  \
                break;                                                                                              \
            vector.elems = backup;                                                                                  \
        }                                                                                                           \
        vector.cap += count;                                                                                        \
    } while (false)

/*
 * Appending element into vector
 * The element MUST be the same type with the vector(<Type>) macro
 * Otherwise, the program might behave strangely
 * 
 * If your vector is null, then 8 room for each one is allocated
 * as defined in VECTOR_STEP, you can change the step before including
 * this header
 *
 * If your vector is full, or vector.cap == vector.len
 * the vector is then reallocated into vector.cap += VECTOR_STEP
 *
 * */
#define vector_push(vector, elem)                                                                                   \
    do {                                                                                                            \
        if (vector.elems == nullptr) {                                                                              \
            vector.elems = calloc(VECTOR_STEP, sizeof(*vector.elems));                                              \
            if (vector.elems == nullptr)                                                                            \
                break;                                                                                              \
            vector.cap += VECTOR_STEP;                                                                              \
        }                                                                                                           \
        else if (vector.cap == vector.len) {                                                                        \
            typeof(vector.elems) tmp = realloc(vector.elems, (vector.cap += VECTOR_STEP) * sizeof(*vector.elems));  \
            if (tmp == nullptr)                                                                                     \
                break;                                                                                              \
            vector.elems = tmp;                                                                                     \
        }                                                                                                           \
        vector.elems[vector.len++] = (typeof(*vector.elems)) elem;                                                  \
    } while (false)

/*
 * Return the pointer to vector.elems[index]
 * If the index >= vector.len, then nullptr is returned
 * 
 * */
#define vector_at(vector, index)                            \
    (vector.len <= index ? nullptr : &vector.elems[index])

/*
 * Freeing the vector
 * Set it back to VECTOR_NEW
 *
 * */
#define vector_free(vector)                     \
    do {                                        \
        free(vector.elems);                     \
        vector = (typeof(vector)) VECTOR_NEW;   \
    } while (false)

/*
 * Dynamic string, alias for vector(char)
 *
 * The difference between C-string
 * 1. It's not null terminated
 * 2. The end of the string is handled by higher level data structure
 * 3. Only accept bytes within ASCII range
 *
 * */
typedef vector(char) string;

/*
 * Same as VECTOR_STEP macro, can be modified before included any header
 * example:
 *
 * #define STRING_STEP 1024
 * #include <libmisc/vector.h>
 * ...
 *
 * */
#ifndef STRING_STEP
#define STRING_STEP 8
#endif

/*
 * Default value for string
 *
 * */
#define STRING_NEW VECTOR_NEW

/*
 * Alias for vector_reserve()
 *
 * */
#define string_reserve(string, count) vector_reserve(string, count)

/*
 * Alias for vector_push()
 *
 * */
#define string_push(string, ch) vector_push(string, ch)

/*
 * Alias for vector_at()
 *
 * */
#define string_at(string, index) vector_at(string, index)

/*
 * Alias for vector_free()
 *
 * */
#define string_free(string) vector_free(string)

/*
 * Appending C-string to string
 * If the string is empty ("\0"), it skipped
 *
 * The resulting length is string.len + strlen(str)
 *
 * */
#define string_pushstr(string, str)                                 \
    do {                                                            \
        size_t len = strlen(str);                                   \
        if (len == 0)                                               \
            break;                                                  \
        if (string.cap == 0)                                        \
            string_reserve(string, len + STRING_STEP);              \
        else if (string.cap - string.len <= len)                    \
            string_reserve(string, string.cap + len + STRING_STEP); \
        if (string.elems == nullptr)                                \
            break;                                                  \
        typeof(string.elems) upwards = string.elems + string.len;   \
        memcpy(upwards, str, len);                                  \
        string.len += len;                                          \
    } while (false)
