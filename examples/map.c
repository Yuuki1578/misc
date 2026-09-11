#define MISC_IMPL
#include "../misc.h"

int main(int argc, const char** argv)
{
    struct hash_map map;
    struct string buffer;
    struct string_view curr, split, save = {0};
    usize largest = 0;

    if (argc == 1) {
        printf("usage: %s <FILE>\n", argv[0]);
        return 1;
    }

    buffer = string_read_path(libc_alloc, argv[1]);
    split = sv_from_string(&buffer, 0, buffer.len);

    hm_init(&map, 1 << 12);
    printf("Readed %zu bytes from file %s\n", buffer.len, argv[1]);

    while (sv_split_by(&split, " ", &curr)) {
        usize *recv, count = 1;

        if (curr.len < 1) continue;

        if ((recv = hm_get(&map, curr.items, curr.len)) == NULL) 
            hm_put(&map, curr.items, curr.len, &count, sizeof count);
        else 
            *recv += 1;
        
    }

    split = sv_from_string(&buffer, 0, buffer.len);
    while (sv_split_by(&split, " ", &curr)) {
        usize *recv;

        if (curr.len < 1) continue;
        recv = hm_get(&map, curr.items, curr.len);
        misc_assert(recv != NULL, "FAILED, please use stb-ds instead");

        if (largest < *recv) {
            largest = *recv;
            save = curr;
        };
    }

    printf("Most common word: \"%.*s\", count: %zu\n",
           string_fmt(save),
           largest);

    array_free(&buffer);
    hm_free(&map);
}
