#define MISC_IMPL
#include "../misc.h"
#define MAX (15)

u8 buffer[MAX];
struct ring_buffer rb;

int main(void)
{
    const char *text = "HELLO";
    usize write_total = 0,
          read_total = 0;

    memset(buffer, 0, sizeof buffer);
    rb = rb_create(buffer, sizeof buffer);

    for (usize i = 0, idx = 0; i < MAX * 5; ++i, idx = i % strlen(text)) {
        char ch;
        write_total += rb_write(&rb, &text[idx], 1);
        read_total += rb_read(&rb, &ch, 1);

        putchar(ch);
        if (ch == 'O')
            putchar('\n');
    }

    printf("Total bytes written to buffer: %zu\n", write_total);
    printf("Total bytes readed from buffer: %zu\n", read_total);
    rb_clear(&rb);
}
