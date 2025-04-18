#include <libmisc/layout.h>
#include <libmisc/string.h>
#include <stdio.h>

void debugPrint(String *string) {
  printf("string: %s\n", string->rawString);
  printf("length: %zu\n", string->length);
  printf("capacity: %zu\n", string->layout.needed);
}

int main(void) {
  String string = StringNew();
  StringReserve(&string, 4096);

  FILE *f = fopen("../test/test.c", "r");

  if (!f) {
    return 1;
  }

  fread(string.rawString, 1, 1000, f);
  debugPrint(&string);

  StringFree(&string);
  fclose(f);
}
