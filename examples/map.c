#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    size_t fail = 0;
    for (size_t i = 0; i < 1024 * 50; i++) {
        char K[16] = { 0 };
        snprintf(K, sizeof K, "%zu", i << 16);
        
        hashkey_t key = { .key = (void *) K, .len = strlen(K) };
        bool ok = hashmap_put(&map, key, &i, sizeof i);
        assert(ok);

        size_t *value = hashmap_get(&map, key);
        if (value != NULL) {
            printf("[%zu] key: \"%s\", value: %zu\n", i, K, *value);
        } else {
            fail++;
        }
        assert(hashmap_delete_at(&map, key));
    }

    printf("retrieve failed count: %zu\n", fail);
    hashmap_free(&map);
    return 0;
}
