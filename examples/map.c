#define MISC_IMPL
#include "../misc.h"

// Word counter

int main(int argc, const char** argv)
{
    if (argc == 1) {
        printfn("usage: %s <FILE>", argv[0]);
        return 1;
    }

    Map map = { 0 };
    String buffer = string_read_path(argv[1]);
    StringView curr, split = sv_from_string(&buffer, 0, buffer.len);
    map_init(&map);

    while (sv_split_by(&split, " \n", &curr)) {
        usize *recv, count = 1;

        if ((recv = map_get(&map, curr.items, curr.len)) == NULL) {
            map_put(&map, curr.items, curr.len, &count, sizeof count);
        } else {
            *recv += 1;
        }
    }

    MapKV pair = { 0 };
    while (map_iterate(&map, &pair)) {
        printfn("Word: '%.*s' = %zu times",
            (int)pair.key_size,
            (char*)pair.key,
            *(usize*)pair.value);
    }

    array_free(&buffer);
    map_free(&map);
}
