#include "../misc.h"

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    char* content = file_read_all(argv[1]);
    if (content != NULL) {
        printf("%s", content);
        free(content);
    }
}
