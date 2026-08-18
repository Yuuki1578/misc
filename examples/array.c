#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Array(i32) ints = { 0 };
    Misc_Allocator* const alloc = misc_mmap_alloc;

    array_extend_with(alloc, &ints, ((int[3]) { 1, 2, 3 }), 3);
    for (i32 i = 0; i > -1024; i--)
        array_append_at_with(alloc, &ints, 0, i);

    array_make_fit_with(alloc, &ints);
    array_reverse(i32, &ints);
    for (usize i = 0; i < ints.len; i++)
        printfn("%zu: %d", i, ints.items[i]);

    printfn("Capacity: %zu, Length: %zu", ints.cap, ints.len);
    array_free_with(alloc, &ints);
}
