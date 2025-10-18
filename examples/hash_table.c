#include "../misc.h"


int main(void)
{
    HashTable table = hashtable_create();
    size_t index;

    printf("%zu\n", (index = misc_FNV1a(HTAB_KEY("Fuck", 4)) % 63));

    HTAB_INSERT(&table, HTAB_KEY("Hello", 5), 0xAB0BA);
    HTAB_INSERT(&table, HTAB_KEY("Fuck", 4), 67);
    HTAB_INSERT(&table, HTAB_KEY("Shit", 4), 69);

    HashRecords *records = vector_get_pos((Vector *) &table, index);
    int *data = records->head->item;
    printf("%x\n", *data);

    (void) table;
}
