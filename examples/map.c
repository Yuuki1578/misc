#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    HashMap map = { 0 };
    size_t fail = 0;

    for (register usize i = 0; i < 1024 * 1024 * 4; i++) {
        usize val = i;
        String buf = string_printf("%zu", i << 2);
        assert(buf.len > 0);
        hashmap_put(
            &map,
            (HashKey) { .key = buf.items, .len = buf.len },
            &val,
            sizeof val);

        array_free(&buf);
    }

    String buf = string_printf("%zu", ((1024 * 698) - 1) << 2);
    HashKey key = {
        .key = buf.items,
        .len = buf.len,
    };

    assert(hashmap_delete_at(&map, key));
    usize* ok = hashmap_get(&map, key);
    assert(ok == NULL);

    printf("retrieve failed count: %zu\n", fail);
    printf("table capacity: %u\n", map.table.cap);
    printf("load factor: %f\n", hashmap_load_factor(&map));

    hashmap_free(&map);
    array_free(&buf);
    return 0;
}
