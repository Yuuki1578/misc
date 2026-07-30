#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Array(i32) ints = {0};
    for (i32 i = 1; i <= 25; i++)
        appendArray(&ints, i * 100);

    i32 additional[] = { -1, -2, -3 };
    extendArray(&ints, additional, 3);
    removeArrayAt(&ints, 0);
    removeArrayAt(&ints, 1);
    removeArrayAt(&ints, ints.len - 1);

    shrinkArrayToFit(&ints);
    for (u64 i = 0; i < ints.len; i++)
        printf("%d\n", ints.items[i]);

    printf("Capacity: %lu\n", ints.cap);
    printf("Length: %lu\n", ints.len);
    freeArray(&ints);
}
