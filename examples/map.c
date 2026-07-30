#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Map map = {0};
    initMap(&map);

    for (usize i = 0; i < 1024 * 1024 * 10; i++) {
        char* key = cstrPrintf("\"%zu\"", i << 4);
        putInMap(&map, key, strlen(key), &i, sizeof i);
        free(key);
    }

    // MapKV pair = {0};
    // while (iterateMap(&map, &pair)) {
    //     char* key = pair.key;
    //     usize* value = pair.value;
    //     printf("Key: %s, Value: %zu\n", key, *value);
    // }

    freeMap(&map);
}
