#include <libmisc/memory.h>
#include <libmisc/string.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

size_t STRING_STEP_HOOK = STRING_ALLOC_STEP;

String string_new(void) {
  return (String){
      .raw_str = nullptr,
      .len = 0,
      .layout = layout_new(sizeof(char), 0),
  };
}

ssize_t string_reserve(String *string, size_t count) {
  Layout *layout;
  void *tmp;

  if (string == nullptr || count == 0) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;

  if (layout->size == 0) {
    layout->size = sizeof(char);
  }

  layout_add(layout, count);

  switch (layout->status) {
  case LAYOUT_NON_NULL:
    tmp = layout_realloc(layout, string->raw_str);
    break;

  default:
    tmp = layout_alloc(layout);
    break;
  }

  if (layout->status != LAYOUT_NON_NULL) {
    return STRING_STATUS_ERR;
  }

  string->raw_str = tmp;
  return count;
}

int string_push(String *string, char ch) {
  char *tmp;

  if (string == nullptr) {
    return STRING_STATUS_ERR;
  }

  if (string->layout.capacity == 0) {
    layout_add(&string->layout, STRING_STEP_HOOK);
    tmp = layout_alloc(&string->layout);
  }

  else if (string->layout.capacity == string->layout.size * string->len) {
    layout_add(&string->layout, STRING_STEP_HOOK);
    tmp = layout_realloc(&string->layout, string->raw_str);
  }

  switch (string->layout.status) {
  case LAYOUT_NULL_PTR:
  case LAYOUT_UNIQUE_PTR:
    return STRING_STATUS_ERR;

  default:
    tmp[string->len++] = ch;
    string->raw_str = tmp;
  }

  return STRING_STATUS_OK;
}

ssize_t string_pushstr(String *string, char *cstr) {
  size_t len;
  char *tmp;

  if (string == nullptr || cstr == nullptr)
    return STRING_STATUS_ERR;

  len = strlen(cstr);

  if (string->layout.capacity > len) {
    memcpy(string->raw_str + string->len, cstr, len);
    string->len += len;

    return len;
  }

  layout_add(&string->layout, len);

  if (string->layout.status == LAYOUT_NULL_PTR)
    tmp = layout_alloc(&string->layout);
  else
    tmp = layout_realloc(&string->layout, string->raw_str);

  if (string->layout.status != LAYOUT_NON_NULL)
    return STRING_STATUS_ERR;

  strncat(tmp, cstr, len);
  string->raw_str = tmp;
  string->len += len;

  return len;
}

char *string_at(String *string, size_t index) {
  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return nullptr;

  if (string->len <= index)
    return nullptr;

  return &string->raw_str[index];
}

void string_crop(String *string) {
  Layout *current;
  char *tmp;

  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return;

  current = &string->layout;

  if (string->len == current->capacity)
    return;

  layout_min(current, current->capacity - string->len);
  tmp = layout_realloc(current, string->raw_str);

  if (current->status != LAYOUT_NON_NULL)
    return;

  string->raw_str = tmp;
}

void string_free(String *string) {
  if (string == nullptr)
    return;

  layout_dealloc(&string->layout, string->raw_str);
}
