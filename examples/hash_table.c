#include "../misc.h"


int main(void)
{
    HashTable table = hashtable_create();
    HashEntry entry = {
        .key.key = "Halo!",
        .key.length = 5,
        .value = &table,
    };
    hashtable_insert(&table, entry);

    entry.key.key = "Howdy rowdy, hash table's here!";
    entry.key.length = strlen(entry.key.key);
    int value = 67;
    entry.value = &value;
    hashtable_insert(&table, entry);

    HashEntry *getent = hashtable_get(&table, entry.key);
    if (getent != NULL)
        printf("%s\n", getent->key.key);

    entry.key.key = "Halo!";
    entry.key.length = strlen(entry.key.key);
    entry.value = MISC_ALLOC(64);

    int *feedme = (int *) entry.value;
    *feedme = 0xfeedface;

    hashtable_insert(&table, entry);

    getent = hashtable_get(&table, entry.key);
    if (getent != NULL) {
        int *repr = (int *) getent->value;
        *repr = 0xbadface;
        printf("%x\n", *repr);
    }

    return 0;
}
