#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Array(i32) ints = { 0 };
    for (i32 i = 1; i <= 25; i++)
        array_append(&ints, i * 100);

    i32 additional[] = { -1, -2, -3 };
    array_extend(&ints, additional, 3);
    array_remove_at(&ints, 0);
    array_remove_at(&ints, 1);
    array_remove_at(&ints, ints.len - 1);

    for (u32 i = 0; i < ints.len; i++)
        printf("%d\n", ints.items[i]);

    array_free(&ints);
}
