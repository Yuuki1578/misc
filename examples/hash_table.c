#include "../misc.h"


int main(void)
{
    HashTable table = hashtable_create();

    hashtable_insert(&table, (HashEntry){
        .key.key = "Halo!",
        .key.length = 5,
        .value = &table,
    });

    char *static_key = "Howdy rowdy, hash table's here!";
    int value = 67;

    HashEntry entry = {
        .key.key = static_key,
        .key.length = strlen(static_key),
        .value = &value,
    };

    HashIndex hashed = misc_FNV1a(entry.key);
    printf("%zu\n", hashed % table.buckets.capacity);
    hashtable_insert(&table, entry);

    return 0;
}
