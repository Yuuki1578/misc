#define MISC_IMPL
#include "../misc.h"
#define MAX (15)

u8 buffer[MAX];
struct ring_buffer rb;

int main(void)
{
    usize write_total = 0,
          read_total = 0;

    rb = rb_init(buffer, sizeof buffer);

    const char *text = "HELLO";
    for (usize i = 0, idx; i < MAX * 5; ++i, idx = i % strlen(text)) {
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
