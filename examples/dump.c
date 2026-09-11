#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    struct hash_map hm;
    int key = 10, value = 20, *getter;

    hm_init(&hm, 1 << 10);
    hm_put(&hm, &key, sizeof(int*), &value, sizeof value);
    getter = hm_get(&hm, &key, sizeof(int*));
    printf("%d\n", *getter);

    hm_free(&hm);
}
