#define MISC_IMPL
#include "../misc.h"
#define TEXT "Hello!"

int main(void)
{
    char *anon_str = mmap_alloc->alloc(NULL, 8, 1);

    if (anon_str != NULL) {
        snprintf(anon_str, 8, TEXT);
        printf("%.*s\n", (int)strlen(TEXT), anon_str);

        anon_str = mmap_alloc->realloc(NULL, anon_str, 5, 1); // "Hello"
        misc_assert(anon_str != NULL, "anon_str == NULL");
        printf("%.*s\n", 5, anon_str);

        mmap_alloc->free(NULL, anon_str);
    }
}
