#define MISC_IMPL
#include "../misc.h"
#define TEXT "Hello!"

int main(void)
{
    char *anon_str = misc_mmap_alloc->alloc(NULL, 8, 1);

    if (anon_str != NULL) {
        snprintf(anon_str, 8, TEXT);
        printfn("%.*s", (int)strlen(TEXT), anon_str);

        anon_str = misc_mmap_alloc->realloc(NULL, anon_str, 5, 1); // "Hello"
        misc_assert(anon_str != NULL, "anon_str == NULL");
        printfn("%.*s", 5, anon_str);

        misc_mmap_alloc->free(NULL, anon_str);
    }
}
