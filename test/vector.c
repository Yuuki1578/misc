#define VECTOR_STEP (1024)
#include "libmisc/vector.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __SIZEOF_INT128__
typedef __uint128_t uint128_t;
#else
typedef unsigned long long uint128_t;
#endif

int main(void)
{
    vector(uint128_t) big_ints = VECTOR_NEW;

    for (int i = 1; i <= UINT16_MAX; i++) {
        vector_push(big_ints, i * 10);
        printf("%lu\n", (uint64_t)big_ints.elems[i - 1]);
    }

    vector_free(big_ints);
    return 0;
}
