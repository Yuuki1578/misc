#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    char buf[] = "HELLO WORLD+HAHA$YEAH";
    StringView view = stringViewFrom(buf, 0, strlen(buf));
    StringView other;

    while (splitStringViewBy(&view, " +$", &other)) {
        printf("%.*s\n", stringFmt(other));
    }
}
