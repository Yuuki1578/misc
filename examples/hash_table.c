#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"


int main(void)
{
    ARENA_INIT();

    HashTable table = table_create();
    HashEntry entry = entry_create("Batak", MISC_ALLOC(64));

    strcpy(entry.value, "Hello, world!");
    if (!table_insert(&table, entry))
        return 1;

    HashEntry *getter = table_get(&table, entry.key);
    if (getter == NULL)
        return 2;

    char *buf = (char *) getter->value;
    printf("%s\n", buf);

    if (table_delete(&table, entry.key))
        if (!table_getvalue(&table, entry.key))
            printf("DELETED!\n");

    return 0;
}
