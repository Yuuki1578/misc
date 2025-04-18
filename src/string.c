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
  char *tmp;

  if (string == nullptr || count == 0) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;
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
  Layout *layout;

  if (string == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;

  if (layout->needed == 0 || layout->needed == layout->size * string->length) {
    ssize_t status = StringReserve(string, StringStepDefault < sizeof(char)
                                               ? sizeof(char)
                                               : StringStepDefault);

    if (status == STRING_STATUS_ERR) {
      return status;
    }
  }

  string->rawString[string->length++] = ch;
  return STRING_STATUS_OK;
}

ssize_t StringPushstr(String *string, char *cstr) {
  Layout *layout;
  size_t length;

  if (string == nullptr || cstr == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;
  length = strlen(cstr);

  if (layout->needed > layout->size * length) {
    memcpy(string->rawString + string->length, cstr, length);
    string->length += length;
    return length;
  }

  ssize_t status = StringReserve(string, length);

  if (status == STRING_STATUS_ERR) {
    return status;
  }

  strncat(string->rawString, cstr, length);
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

int StringCrop(String *string) {
  Layout *current;
  char *temporary;

  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return STRING_STATUS_ERR;

  current = &string->layout;

  if (string->length == current->needed)
    return STRING_STATUS_OK;

  LayoutMin(current, current->needed - string->length);
  temporary = LayoutRealloc(current, string->rawString);

  if (current->status != LAYOUT_NON_NULL)
    return STRING_STATUS_ERR;

  string->rawString = temporary;
  return STRING_STATUS_OK;
}

void StringFree(String *string) {
  if (string == nullptr)
    return;

  LayoutDealloc(&string->layout, string->rawString);
}
