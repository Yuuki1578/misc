#include "../Misc.h"

int main(void) {
  RawDlink *Link = rawDlinkCreate(ADDRESS_OF(0), sizeof(int)), *Save;

  if (Link == NULL)
    return 1;

  Save = Link;
  rawDlinkPrepend(&Link, ADDRESS_OF(-1), sizeof(int));
  rawDlinkPrepend(&Link, ADDRESS_OF(-2), sizeof(int));
  rawDlinkPrepend(&Link, ADDRESS_OF(-3), sizeof(int));

  Link = Save;
  rawDlinkAppend(&Link, ADDRESS_OF(1), sizeof(int));
  rawDlinkAppend(&Link, ADDRESS_OF(2), sizeof(int));
  rawDlinkAppend(&Link, ADDRESS_OF(3), sizeof(int));

  for (typeof(Link) Current = rawDlinkRewind(&Link); Current != NULL;
       Current = Current->Next) {
    int *Item = Current->Item;
    printf("%d\n", *Item);
  }

  rawDlinkFree(Link);
}
