#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    u8 buf[128];
    RingBuffer rb = initRbFrom(buf, sizeof buf);
    clearRb(&rb);

    const char *word = "Hello!";
    writeToRb(&rb, word, strlen(word));

    char copy[7];
    readFromRb(&rb, copy, sizeof copy - 1);
    copy[sizeof copy - 1] = 0;
    printfn("%s", buf);
}
