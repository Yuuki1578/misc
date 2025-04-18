#include <libmisc/layout.h>
#include <libmisc/string.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

size_t StringStepDefault = STRING_ALLOC_STEP;

String StringNew(void) {
  return (String){
      .rawString = nullptr,
      .length = 0,
      .layout = LayoutNew(sizeof(char), 0),
  };
}

ssize_t StringReserve(String *string, size_t count) {
  Layout *layout;
  void *tmp;

  if (string == nullptr || count == 0) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  LayoutAdd(layout, count);

  switch (layout->status) {
  case LAYOUT_NON_NULL:
    tmp = LayoutRealloc(layout, string->rawString);
    break;

  default:
    tmp = LayoutAlloc(layout);
    break;
  }

  if (layout->status != LAYOUT_NON_NULL) {
    return STRING_STATUS_ERR;
  }

  string->rawString = tmp;
  return count;
}

int StringPush(String *string, char ch) {
  char *temporary;

  if (string == nullptr) {
    return STRING_STATUS_ERR;
  }

  if (string->layout.capacity == 0) {
    LayoutAdd(&string->layout, StringStepDefault);
    temporary = LayoutAlloc(&string->layout);
  }

  else if (string->layout.capacity == string->layout.size * string->length) {
    LayoutAdd(&string->layout, StringStepDefault);
    temporary = LayoutRealloc(&string->layout, string->rawString);
  }

  switch (string->layout.status) {
  case LAYOUT_NULL_PTR:
  case LAYOUT_UNIQUE_PTR:
    return STRING_STATUS_ERR;

  default:
    temporary[string->length++] = ch;
    string->rawString = temporary;
  }

  return STRING_STATUS_OK;
}

ssize_t StringPushstr(String *string, char *cstr) {
  size_t length;
  char *temporary;

  if (string == nullptr || cstr == nullptr)
    return STRING_STATUS_ERR;

  length = strlen(cstr);

  if (string->layout.capacity > length) {
    memcpy(string->rawString + string->length, cstr, length);
    string->length += length;

    return length;
  }

  LayoutAdd(&string->layout, length);

  if (string->layout.status == LAYOUT_NULL_PTR)
    temporary = LayoutAlloc(&string->layout);
  else
    temporary = LayoutRealloc(&string->layout, string->rawString);

  if (string->layout.status != LAYOUT_NON_NULL)
    return STRING_STATUS_ERR;

  strncat(temporary, cstr, length);
  string->rawString = temporary;
  string->length += length;

  return length;
}

char *StringAt(String *string, size_t index) {
  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return nullptr;

  if (string->length <= index)
    return nullptr;

  return &string->rawString[index];
}

void StringCrop(String *string) {
  Layout *current;
  char *temporary;

  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return;

  current = &string->layout;

  if (string->length == current->capacity)
    return;

  LayoutMin(current, current->capacity - string->length);
  temporary = LayoutRealloc(current, string->rawString);

  if (current->status != LAYOUT_NON_NULL)
    return;

  string->rawString = temporary;
}

void StringFree(String *string) {
  if (string == nullptr)
    return;

  LayoutDealloc(&string->layout, string->rawString);
}
