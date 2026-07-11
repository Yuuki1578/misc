#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    for (size_t i = 0; i < 1024; i++) {
        const size_t K = i << 4;
        hashkey_t key = {
            .key = (void*) &K,
            .len = sizeof K,
        };

        bool ok = hashmap_put(
            &map,
            key,
            &i,
            sizeof i);
        assert(ok);

        size_t *value = hashmap_get(
            &map,
            key);

        if (value != NULL) {
            printf("[%zu]: value is: %zu\n", i, *value);
        }
    }

    hashmap_free(&map);
    return 0;
}
