#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    HashMap* map = hm_init(64);
    Arena* arena = arena_init(1 << 16);

    for (usize i = 0; i < 10; i++) {
        char* key = cstring_printf(arena, "%zu", i << 4);
        usize* value = arena_alloc(arena, sizeof i);
        *value = i;

        hm_put(map, key, strlen(key), value);
    }

    for (usize i = 0; i < 10; i++) {
        char* key = cstring_printf(arena, "%zu", i << 4);
        usize* v = hm_get(map, key, strlen(key));
        printf("%zu\n", v != NULL ? *v : 512);
    }

    hm_free(map);
}
