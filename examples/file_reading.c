#include "../misc.h"
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

    fclose(handler);
    return buffer;
}

int main(void)
{
    Arena* allocator_context = arena_create(ARENA_PAGE);
    char* file_content = read_entire_file(allocator_context, __FILE__);

    printf("%s", file_content);

    arena_alloc(allocator_context, 4000);
    printf("Total size of arena: %zu\n"
           "Offset: %zu\n"
           "Remains: %zu\n",
        allocator_context->next->total,
        allocator_context->next->offset,
        remain_of(allocator_context->next));

    arena_alloc(allocator_context, 4000);
    printf("Total size of arena: %zu\n"
           "Offset: %zu\n"
           "Remains: %zu\n",
        allocator_context->next->next->total,
        allocator_context->next->next->offset,
        remain_of(allocator_context->next->next));

    arena_free(allocator_context);
}
