#define MISC_IMPL
#include "../misc.h"

int main(int argc, const char** argv)
{
    struct hash_map map = {0};
    struct hm_pair pair = {0};
    struct string buffer;
    struct string_view curr, split;

    if (argc == 1) {
        printf("usage: %s <FILE>\n", argv[0]);
        return 1;
    }

    buffer = string_read_path(libc_alloc, argv[1]);
    split = sv_from_string(&buffer, 0, buffer.len);
    hm_init(&map, 2);

    while (sv_split_by(&split, " \n", &curr)) {
        usize *recv, count;
        count = 1;

        if ((recv = hm_get(&map, curr.items, curr.len)) == NULL) 
            hm_put(&map, curr.items, curr.len, &count, sizeof count);
        else 
            *recv += 1;
        
    }

    while (hm_iterate(&map, &pair)) {
        printf("Word: '%.*s' = %zu times\n",
            (int)pair.key_size,
            (char*)pair.key,
            *(usize*)pair.value);
    }

    array_free(&buffer);
    hm_free(&map);
}
