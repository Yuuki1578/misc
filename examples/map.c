#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    HashMap map = { 0 };
    Arena* allocator = arena_init(1 << 12, MISC_ARHEAP);
    size_t fail = 0;
    assert(allocator != NULL);

    for (usize i = 0; i < 1024 * 1024; i++) {
        char* buf = cstring_printf(allocator, "%zu", i << 2);
        assert(buf != NULL);

        hashmap_put_cstr(&map, buf, &i, sizeof i);
        if (!hashmap_get_cstr(&map, buf))
            fail++;
    }

    char* key = cstring_printf(allocator, "%zu", ((1024 * 698) - 1) << 2);
    const HashKey K = { .key = (void*)key, .len = strlen(key) };
    assert(hashmap_delete_at(&map, K));
    usize* ok = hashmap_get(&map, K);
    if (ok == NULL)
        goto skip;

    printf("retrieved: %zu\n", *ok);

skip:;
    printf("retrieve failed count: %zu\n", fail);
    printf("table capacity: %u\n", map.table.cap);
    printf("load factor: %f\n", hashmap_loadfactor(&map));

    hashmap_free(&map);
    arena_free(allocator);
    return 0;
}
