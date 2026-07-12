#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    size_t fail = 0;
    for (size_t i = 0; i < 1024 * 50; i++) {
        const size_t K = i << 4;
        bool ok;
        hashmap_insert(&map, K, i, &ok);
        assert(ok);

        size_t *value;
        hashmap_retrieve(&map, K, &value);
        if (value != NULL) {
            printf("[%zu] key: %zu, value: %zu\n", i, K, *value);
        } else {
            fail++;
        }
    }

    printf("retrieve failed count: %zu\n", fail);
    hashmap_free(&map);
    return 0;
}
