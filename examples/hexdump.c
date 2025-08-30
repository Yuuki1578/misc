#define MISC_USE_GLOBAL_ALLOCATOR
#define VECTOR_ALLOC_FREQ (1024)
#include "../misc.h"
#include <errno.h>

#if defined(__LP64__) || defined(__LLP64__)
#define PRINTABLE_SPAN (16)
#else
#define PRINTABLE_SPAN (8)
#endif

void visit_byte(char* content, size_t length, size_t span);
void mode_stdin(size_t span);

int main(int argc, char** argv)
{
    ARENA_INIT();

    if (argc == 1) {
        mode_stdin(PRINTABLE_SPAN);
    } else {
        char* file_content = file_read_all(argv[1]);
        if (file_content == NULL) {
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            misc_free();

            return 1;
        }

        visit_byte(file_content, strlen(file_content) + 1, PRINTABLE_SPAN);
    }

    misc_free();
}

void visit_byte(char* content, size_t length, size_t span)
{
    for (size_t i = 0, j = 0; i < length; i++) {
        if (j != span) {
            printf("%02X ", content[i]);
            j++;
        } else {
            putchar('\n');
            j = 0;
        }
    }

    putchar('\n');
}

void mode_stdin(size_t span)
{
    String string = string_with(1024);
    int ch;

    while ((ch = fgetc(stdin)) != EOF)
        string_push(&string, ch);

    visit_byte(vector_at((void*)&string, 0), string.vector.length + 1, span);
}
