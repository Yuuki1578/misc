#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    /*
    Generic! though, it's anonymous struct
    it would be kinda cool if anonymous struct is identified
    based on their layout, so it can be passed around without taking
    the address and convert it to void* like zig does, like:

    struct {
        int A;
        const char* B;
    } reverseLayout(struct {
        const char* A;
        int B;
    } reversed) {
        return (typeof(reverseLayout(reversed))){
            .A = reversed.B,
            .B = reversed.A,
        };
    }
    */
    Array(i32) ints = {0};

    for (i32 i = 0; i > -32; i--)
        appendArrayAt(&ints, 0, i);

    shrinkArrayToFit(&ints);
    reverseArray(i32, &ints);
    for (usize i = 0; i < ints.len; i++)
        printfn("%zu: %d", i, ints.items[i]);

    printfn("Capacity: %zu, Length: %zu", ints.cap, ints.len);
    freeArray(&ints);
}
