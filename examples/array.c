#define MISC_IMPL
#include "../misc.h"

struct many_int {
    i32 *items;
    usize cap, len;
};

int main(void)
{
    struct many_int ints = { 0 };
    struct allocator *const alloc = mmap_alloc;

    array_extend_with(alloc, &ints, ((int[3]) { 1, 2, 3 }), 3);
    for (i32 i = 0; i > -1024; i--)
        array_append_at_with(alloc, &ints, 0, i);

    array_make_fit_with(alloc, &ints);
    array_reverse(i32, &ints);

    for (usize i = 0; i < ints.len; i++)
        printf("%zu: %d\n", i, ints.items[i]);

    printf("Capacity: %zu, Length: %zu\n", ints.cap, ints.len);
    array_free_with(alloc, &ints);
}
