#define MISC_IMPL
#include "../misc.h"

// Word counter

int main(int argc, const char** argv)
{
    if (argc == 1) {
        printfn("usage: %s <FILE>", argv[0]);
        return 1;
    }

    Hash_Map map = { 0 };
    String buffer = string_read_path(argv[1]);
    String_View curr, split = sv_from_string(&buffer, 0, buffer.len);
    hm_init(&map, 1024);

    while (sv_split_by(&split, " \n", &curr)) {
        usize *recv, count = 1;

        if ((recv = hm_get(&map, curr.items, curr.len)) == NULL) {
            hm_put(&map, curr.items, curr.len, &count, sizeof count);
        } else {
            *recv += 1;
        }
    }

    Hash_Map_KV pair = { 0 };
    while (hm_iterate(&map, &pair)) {
        printfn("Word: '%.*s' = %zu times",
            (int)pair.key_size,
            (char*)pair.key,
            *(usize*)pair.value);
    }

    array_free(&buffer);
    hm_free(&map);
}
