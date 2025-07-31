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

#ifndef MISC_LIST_H
#define MISC_LIST_H

#include <stddef.h>
#include <stdlib.h>

#define MISC_LIST_FREQ (8ULL)

/* NOTE:
The macros below are works on all struct implementing the field listed here:
1. <Type> *items.
2. size_t capacity.
3. size_t length.

Where <Type> are some concrete types. */

/* Anonymous struct, a generic-like type for C.
Examples:

    int main(void)
    {
        List(int) some_integer = {0};
        List(List(char*)) some_list_of_list_of_string = {0};

        list_append(some_integer, 10);
    }

*/
#define List(T)          \
    struct {             \
        T* items;        \
        size_t capacity; \
        size_t length;   \
    }

/* Resizing the list up and/or down. */
#define list_resize(list, size)                                                                                     \
    do {                                                                                                            \
        if ((list).capacity < 1 || (list).items == NULL) {                                                          \
            (list).items = (typeof((list).items))calloc((size), sizeof(typeof(*(list).items)));                     \
            if ((list).items == NULL)                                                                               \
                break;                                                                                              \
        } else {                                                                                                    \
            typeof((list).items) tmp = (typeof((list).items))realloc((list).items, (size) * sizeof(*(list).items)); \
            if ((list).items == NULL)                                                                               \
                break;                                                                                              \
            (list).items = tmp;                                                                                     \
        }                                                                                                           \
        if ((size) < (list).length)                                                                                 \
            (list).length = size;                                                                                   \
        (list).capacity = size;                                                                                     \
    } while (0)

/* Make the list fitting to it's length. */
#define list_make_fit(list)                                        \
    do {                                                           \
        if ((list).length < 1 || (list).capacity == (list).length) \
            break;                                                 \
        list_resize(list, (list).length);                          \
    } while (0)

/* Append an item to the list, increasing it's length. */
#define list_append(list, item)                                        \
    do {                                                               \
        if ((list).capacity < 1) {                                     \
            list_resize(list, MISC_LIST_FREQ);                         \
        } else if ((list).capacity - (list).length <= 1) {             \
            list_resize(list, (list).capacity * 2);                    \
        }                                                              \
        (list).items[(list).length++] = (typeof(*(list).items))(item); \
    } while (0)

/* Freeing the list, truncating it's capacity to zero. */
#define list_free(list)                                  \
    do {                                                 \
        if ((list).capacity > 0 || (list).items != NULL) \
            free((list).items);                          \
        (list).capacity = 0;                             \
        (list).length = 0;                               \
    } while (0)

#endif
