#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Map map = {0};
    initMap(&map);
    Arena* arena = initArena(1 << 12);

    for (usize i = 0; i < 1024*1024; i++) {
        char* key = cstrArenaPrintf(arena, "%zu", i);
        putInMap(&map, key, strlen(key), &i, sizeof i);
    }

    for (usize i = 0; i < 1024*1024; i++) {
        char* key = cstrArenaPrintf(arena, "%zu", i);
        usize* value = getFromMap(&map, key, strlen(key));
        printf("%zu\n", *value);
    }

    // for (usize i = 0; i < 1024*1024; i++) {
    //     String key = stringPrintf("%zu", i);
    //     deleteFromMap(&map, key.items, key.len);
    //     freeArray(&key);
    // }

    // for (usize i = 0; i < 1024*1024; i++) {
    //     String key = stringPrintf("%zu", i);
    //     putInMap(&map, key.items, key.len, &i, sizeof i);
    //     freeArray(&key);
    //     // printf("'%.*s': %zu\n", stringFmt(key), *value);
    // }

    freeMap(&map);
}
