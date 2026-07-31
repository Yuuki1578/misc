#define MISC_IMPL
#include "../misc.h"
#include <errno.h>

// Word counter

int main(int argc, const char **argv)
{
    if (argc == 1) {
        printfn("usage: %s <FILE>", argv[0]);
        return 1;
    }

    Map map = {0};
    String buffer = readFileToString(argv[1]);
    StringView curr, split = initSvFromString(&buffer, 0, buffer.len);
    initMap(&map);

    while (splitSvBy(&split, " \n", &curr)) {
        usize *recv, count = 1;

        if ((recv = getFromMap(&map, curr.items, curr.len)) == NULL) {
            putInMap(&map, curr.items, curr.len, &count, sizeof count);
        } else {
            *recv += 1;
        }
    }

    MapKV pair = {0};
    while (iterateMap(&map, &pair)) {
        printfn("Word: '%.*s' = %zu times",
               (int)pair.keyLen,
               (char*)pair.key,
               *(usize*)pair.value);
    }

    freeArray(&buffer);
    freeMap(&map);
}
