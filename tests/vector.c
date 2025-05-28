#include <libmisc/vector.h>
#include <stdio.h>

int main(void) {
  string str = STRING_NEW;
  string_pushstr(str, "Hello, world!\n");
  string_pushstr(str, "My name is Awang!\n");
  string_pushstr(str, "Nice to meet ya");

  printf("%s\n", str.elems);
  string_free(str);
}
