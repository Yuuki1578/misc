#define MISC_IMPL
#include "../misc.h"

int main(int argc, const char **argv) {
  if (argc < 2) {
    printf("usage: %s <FILE>\n", argv[0]);
    return 1;
  }

  String fromFile = stringReadPath(argv[1]);
  StringView view = {fromFile.items, fromFile.len};
  StringView current = {0};
  HashMap map;
  hashMapInit(&map, 1 << 12);

  while (viewSplitBy(&view, " ", &current)) {
    if (current.len < 1)
      continue;

    usize *wordCount = hashMapGet(&map, current.items, current.len);
    if (!wordCount) {
      usize initial = 1;
      hashMapPut(&map, current.items, current.len, &initial, sizeof initial);
    } else {
      *wordCount += 1;
    }
  }

  HashPair pair = {0};
  StringView mostWord;
  usize wordCount = 0;

  while (hashMapIterate(&map, &pair)) {
    const usize *count = pair.value;
    if (*count > wordCount) {
      wordCount = *count;
      mostWord = (StringView){pair.key, pair.keySize};
    }
  }

  printf("Word with the most count: \"%.*s\", count: %zu\n",
         stringFmt(mostWord), wordCount);
  arrayFree(&fromFile);
  hashMapFree(&map);
}
