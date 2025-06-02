#define VECTOR_STEP 1024
#define STRING_STEP VECTOR_STEP

#include <libmisc/vector.h>
#include <stdio.h>

int main(void) {
  vector_new(int, ints);

  for (int i = 0, j = 1; i < 100; i++, j++) {
    vector_push(ints, i * 100);
  }

  vector_shrink_to_fit(ints);
  printf("%zu\n", ints.cap);
  printf("%zu\n", ints.len);

  vector_free(ints);

  String str = STRING_NEW;
  string_pushstr(str, "My file located in: ");
  string_pushstr(str, __FILE__);

  printf("%zu\n", str.cap);
  string_shrink_to_fit(str);
  printf("%s\n", str.elems);
  printf("%zu\n", str.cap);
  printf("%zu\n", str.len);
}
