#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"
#include <ctype.h>


const char *poetry =
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n"
    "START DIGGIN IN YO BUTT TWIN\n";

int main(void)
{
    ARENA_INIT();

    HashTable table = table_create();
    VECTOR(char) word = {0};
    VECTOR(char *) saves = {0};

    for (size_t i = 0; i < strlen(poetry); i++) {
        char ch = poetry[i];
        if (!isspace(ch)) {
            VECTOR_PUSH(word, ch);
            continue;
        }

        HashEntry entry = entry_create(word.items, MISC_ALLOC(sizeof(unsigned int)));
        unsigned int *count = (unsigned int *) table_get_value(&table, entry.key);

        if (count == NULL) {
            table_insert(&table, entry);
            count = (unsigned int *) entry.value;
            *count = 1;
        } else {
            *count += 1;
        }

        char *save_word = MISC_ALLOC(word.length + 1);
        strncpy(save_word, word.items, word.length);

        VECTOR_PUSH(saves, save_word);
        VECTOR_RESIZE(word, 0);
    }

    for (size_t i = 0; i < saves.length; i++) {
        char *s = saves.items[i];
        unsigned int *count = (void *) table_get_value(&table, key_create(s));

        printf("%s: %u\n", s, *count);
    }
}
