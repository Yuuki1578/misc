#include "../misc.h"
#include <stdio.h>
#include <stdlib.h>

char *read_entire_file(Arena *allocator, const char *path)
{
    FILE *stream = fopen(path, "rb");
    if (!stream)
        exit(1);

    char *buf = arena_alloc(allocator, BUFSIZ);
    size_t offset = 0, size = BUFSIZ;

    while (buf) {
        size_t readed;
        if ((readed = fread(buf + offset, 1, BUFSIZ - 1, stream)) == 0)
            break;

        offset += readed;
        buf = arena_realloc(allocator, buf, size, size * 2);
        size *= 2;
    }

    fclose(stream);
    return buf;
}

int main(void)
{
    Arena *context = arena_create(ARENA_PAGE);
    char *buf = read_entire_file(context, __FILE__);

    printf("%s", buf);

    arena_alloc(context, 4000);
    printf("Total size of arena: %zu\n"
           "Offset: %zu\n"
           "Remains: %zu\n",
           context->next->total, context->next->offset,
           REMAIN_OF(context->next));

    arena_alloc(context, 4000);
    printf("Total size of arena: %zu\n"
           "Offset: %zu\n"
           "Remains: %zu\n",
           context->next->next->total,
           context->next->next->offset,
           REMAIN_OF(context->next->next));

    arena_free(context);
}
