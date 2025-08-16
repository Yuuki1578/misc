#include "../include/libmisc/file.h"
#include <stdlib.h>

static char* read_from_stream(FILE* file)
{
    size_t bufsiz = BUFSIZ + 1, readed = 0;
    char* buffer;

    if (file == NULL)
        return NULL;

    if ((buffer = calloc(1, bufsiz)) == NULL)
        return NULL;

    while ((readed += fread(buffer + readed, 1, bufsiz, file)) > 0) {
        char* temporary = realloc(buffer, bufsiz * 2);
        if (temporary == NULL)
            return buffer;

        buffer = temporary;
        bufsiz *= 2;
    }

    buffer = realloc(buffer, readed + 1);
    buffer[readed] = '\0';
    return buffer;
}

char* file_readall(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file != NULL) {
        char* buffer = read_from_stream(file);
        fclose(file);

        if (buffer != NULL)
            return buffer;
    }
    return NULL;
}

char* file_readfrom(FILE* file)
{
    return read_from_stream(file);
}
