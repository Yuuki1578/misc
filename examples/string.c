#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    char buf[] = "#$& HELLO #$&";
    struct string_view view = sv_from(buf, 0, strlen(buf));
    struct string_view other = sv_trim_by(&view, " #$&");
    struct string newer = string_printf(libc_alloc, "'%.*s'", string_fmt(other));

    string_to_lower(&newer);
    array_reverse(char, &newer);
    printfn("%.*s", string_fmt(newer));
}
