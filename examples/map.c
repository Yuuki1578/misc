#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    ChainMap map = {0};
    u64 fail = 0;

    for (u64 i = 0; i < 1024 * 1024 * 10; i++) {
        u64 val = i;
        String buf = string_printf("%lu", i << 2);
        assert(buf.len > 0);
        chainmap_put(&map,
            (HashKey) {
                .key = buf.items,
                .len = buf.len,
            },
            &val, sizeof val);

        array_free(&buf);
    }

    String buf = string_printf("%lu", ((1024 * 698) - 1) << 2);
    HashKey key = {
        .key = buf.items,
        .len = buf.len,
    };

    u64* ok = chainmap_get(&map, key);
    assert(ok != NULL);
    *ok = 0xdeadbee;
    ok = chainmap_get(&map, key);
    printf("%0lx\n", *ok);

    printf("retrieve failed count: %zu\n", fail);
    printf("table capacity: %lu\n", map.table.cap);
    printf("load factor: %f\n", chainmap_load_factor(&map));

    array_free(&buf);
    chainmap_free(&map);
    return 0;
}
