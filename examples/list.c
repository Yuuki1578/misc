#include "../misc.h"
#include <stdio.h>

typedef List(long) LongList;

void print_list(LongList* source)
{
    printf("%zu\n", source->capacity);
    printf("%zu\n", source->length);
    list_make_fit(*source);

    printf("%zu\n", source->capacity);
    printf("%zu\n", source->length);
}

int main(void)
{
    LongList some_list = { 0 };

    for (long i = 0, j = 1; i < 10000; i++, j++) {
        list_append(some_list, j * 10);
        printf("%li\n", some_list.items[i]);
    }

    print_list((void*)&some_list);
    list_free(some_list);
}
