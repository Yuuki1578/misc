#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Array(int) ints = {0};
    for (int i = 1; i <= 25; i++)
        AppendArray(&ints, i * 100);

    int additional[] = {-1, -2, -3};
    ExtendArray(&ints, additional, 3);
    RemoveFromArray(&ints, 0);
    RemoveFromArray(&ints, 1);
    RemoveFromArray(&ints, ints.len - 1);

    for (uint32_t i = 0; i < ints.len; i++)
        printf("%d\n", ints.items[i]);

    FreeArray(&ints);
}
