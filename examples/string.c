#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    char buf[] = "#$& HELLO #$&";
    StringView view = initSvFrom(buf, 0, strlen(buf));
    StringView other = trimSvBy(&view, " #$&");
    String newer = stringPrintf("'%.*s'", stringFmt(other));

    toStringLowercase(&newer);
    reverseArray(char, &newer);
    printfn("%.*s", stringFmt(newer));
}
