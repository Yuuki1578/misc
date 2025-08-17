#include "../misc.h"
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    Vector vector = vector_with(1, sizeof(size_t));

    for (size_t i = 1 << 14; i > 0; i--)
        vector_push(&vector, &i);

    for (size_t i = 0, *current; i < vector.length; i++) {
        current = vector_at(&vector, i);
        printf("%zu\n", *current);
    }

    printf("Capacity:  %zu\n", vector.capacity);
    printf("Length:    %zu\n", vector.length);
    printf("Remaining: %zu\n", vector_remaining(&vector));
    vector_free(&vector);
    return 0;
}
