#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    arena_t* allocator = arena_init(1 << 12, MISC_ARHEAP);
    size_t fail = 0;
    assert(allocator != NULL);

    for (size_t i = 0; i < 1024 * 100; i++) {
        char* buf = str_printf(allocator, "%zu", i << 16);
        assert(buf != NULL);

        hashkey_t key = { .key = (void*)buf, .len = strlen(buf) };
        bool ok = hashmap_put(&map, key, &i, sizeof i);
        assert(ok);

        size_t* value = hashmap_get(&map, key);
        if (value != NULL)
            printf("[%zu] key: \"%s\", value: %zu\n", i, buf, *value);
        else
            fail++;

        assert(hashmap_delete_at(&map, key));
    }

    hashmap_free(&map);
    arena_free(allocator);

    printf("retrieve failed count: %zu\n", fail);
    return 0;
}
