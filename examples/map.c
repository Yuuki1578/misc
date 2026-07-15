#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    arena_t* allocator = arena_init(1 << 12, MISC_ARHEAP);
    size_t fail = 0;
    assert(allocator != NULL);

    for (size_t i = 0; i < 1024 * 100; i++) {
        char* buf = cstr_printf(allocator, "%zu", i << 8);
        assert(buf != NULL);

        hashmap_put_cstr(&map, buf, &i, sizeof i);
        if (!hashmap_get_cstr(&map, buf))
            fail++;
    }

    char* key = cstr_printf(allocator, "%zu", ((1024 * 100) - 1) << 8);
    size_t* ok = hashmap_get(&map, (hashkey_t) { .key = (void*)key, .len = strlen(key) });
    if (!ok)
        return 1;

    printf("retrieved: %zu\n", *ok);
    printf("retrieve failed count: %zu\n", fail);
    printf("table capacity: %u\n", map.table.cap);
    printf("load factor: %f\n", hashmap_loadfactor(&map));

    hashmap_free(&map);
    arena_free(allocator);
    return 0;
}
