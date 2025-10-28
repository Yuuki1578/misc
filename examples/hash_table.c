#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"


const char *place[] = {
    [HP_InBuckets] = "InBuckets",
    [HP_InEntries] = "InEntries",
    [HP_NotFound] = "NotFound",
};

int main(void)
{
    ARENA_INIT();

    HashTable table = table_create();
    HashEntry entry = entry_create("Batak", MISC_ALLOC(64));

    strcpy(entry.value, "Hello, world!");
    if (!table_insert(&table, entry))
        return 1;

    char *value = (char *) table_get_value(&table, entry.key);
    if (value != NULL) {
        printf("%s\n", value);

        // No need to free it, since it managed by arena.
        // free(value);
    }

    table_remove(&table, entry.key);
    value = (char *) table_get_value(&table, entry.key);
    if (value == NULL)
        printf("DELETED!\n");

    return 0;
}
