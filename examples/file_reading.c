#include "../include/libmisc/arena.h"
#include <stdio.h>
#include <stdlib.h>

char* read_entire_file(Arena* allocator, const char* path)
{
    FILE* handler = fopen(path, "rb");
    if (!handler)
        exit(1);

    char* buffer = arena_alloc(allocator, BUFSIZ);
    size_t offset = 0, size = BUFSIZ;

    while (buffer) {
        size_t readed;
        if ((readed = fread(buffer + offset, 1, BUFSIZ - 1, handler)) == 0)
            break;

        offset += readed;
        buffer = arena_realloc(allocator, buffer, size, size * 2);
        size *= 2;
    }

    return buffer;
}

int main(void)
{
    Arena* allocator_context = arena_create(ARENA_PAGE);
    char* file_content = read_entire_file(allocator_context, __FILE__);

    printf("%s", file_content);
    arena_free(allocator_context);
}
