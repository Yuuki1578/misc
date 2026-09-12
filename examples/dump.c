#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    double set[] = {8, 8, 10, 11, 15, 17};
    printf("%f\n", set_median(set, 6));
    printf("%f\n", set_mode(set, 6));
    printf("%f\n", set_range(set, 6));
}
