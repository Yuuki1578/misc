#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    struct { u64 key; u64 value; } *map = NULL;

    for (u64 i = 0; i < 1024 * 1024 * 10; i++) {
        u64 val = i;
        map_put(&map, i << 2, val);
    }

    map_delete_at(&map, (1024 * 698) << 2);
    u64* val;
    map_get(&map, (1024 * 698) << 2, &val);
    assert(val == NULL);

    map_get(&map, (1024 * 1024 * 5) << 2, &val);
    assert(val != NULL);
    printf("%lu\n", *val);

    map_free(&map);
    return 0;
}
