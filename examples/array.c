#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    array_t(int) ints = {0};
    for (int i = 1; i <= 25; i++)
        array_append(&ints, i * 100);

    int additional[] = {-1, -2, -3};
    array_extend(&ints, additional, 3);
    array_remove_at(&ints, 0);
    array_remove_at(&ints, 1);
    array_remove_at(&ints, ints.len - 1);

    for (uint32_t i = 0; i < ints.len; i++)
        printf("%d\n", ints.items[i]);

    array_free(&ints);
}
