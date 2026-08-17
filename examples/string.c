#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    char buf[] = "#$& HELLO #$&";
    StringView view = sv_from(buf, 0, strlen(buf));
    StringView other = sv_trim_by(&view, " #$&");
    String newer = string_printf("'%.*s'", string_fmt(other));

    string_to_lower(&newer);
    array_reverse(char, &newer);
    printfn("%.*s", string_fmt(newer));
}
