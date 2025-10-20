#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"


void exit_hook(void)
{
    printf("DONE\n");
    ARENA_DEINIT();
}

int main(void)
{
    ARENA_INIT();
    atexit(exit_hook);

    HashTable table = hashtable_create();
    HashEntry entry = {
        .key.key = "Batak",
        .key.length = 5,
        .value = MISC_ALLOC(64),
    };

    strcpy(entry.value, "Hello, world!");
    if (!hashtable_insert(&table, entry))
        return 1;

    HashEntry *getter = hashtable_get(&table, entry.key);
    if (getter == NULL)
        return 2;

    char *buf = (char *) getter->value;
    printf("%s\n", buf);

    if (hashtable_delete(&table, entry.key))
        if (!hashtable_get(&table, entry.key))
            printf("DELETED!\n");

    ARENA_DEINIT();
}
