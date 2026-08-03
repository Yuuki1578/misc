#define MISC_IMPL
#include "../misc.h"
#define MAX (15)

u8 buffer[MAX];
RingBuffer rb;

int main(void)
{
    usize writeTotal = 0,
          readTotal  = 0;

    rb = initRbFrom(buffer, sizeof buffer);

    const char* text = "HELLO";
    for (usize i = 0, idx; i < MAX * 5; ++i, idx = i % strlen(text)) {
        char ch;
        writeTotal += writeToRb(&rb, &text[idx], 1);
        readTotal += readFromRb(&rb, &ch, 1);

        putchar(ch);
        if (ch == 'O')
            putchar('\n');
    }

    printfn("Total bytes written to buffer: %zu", writeTotal);
    printfn("Total bytes readed from buffer: %zu", readTotal);
    clearRb(&rb);
}
