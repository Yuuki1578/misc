#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    char buf[] = "#$& HELLO #$&";
    String_View view = sv_from(buf, 0, strlen(buf));
    String_View other = sv_trim_by(&view, " #$&");
    String newer = string_printf(misc_libc_alloc, "'%.*s'", string_fmt(other));

    string_to_lower(&newer);
    array_reverse(char, &newer);
    printfn("%.*s", string_fmt(newer));
}
