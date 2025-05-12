// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file string.c
// @brief dynamically allocated string type

#include <libmisc/layout.h>
#include <libmisc/string.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

size_t StringStepDefault = STRING_ALLOC_STEP;

String string_new(void) {
  return (String){
      .rawptr = nullptr,
      .length = 0,
      .layout = layout_new(sizeof(char), 0),
  };
}

ssize_t string_reserve(String* string, size_t count) {
  Layout* layout;
  char* tmp;

  if (string == nullptr || count == 0) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;
  layout_add(layout, count);

  switch (layout->status) {
    case LAYOUT_NON_NULL:
      tmp = layout_realloc(layout, string->rawptr);
      break;

    default:
      tmp = layout_alloc(layout);
      break;
  }

  if (layout->status != LAYOUT_NON_NULL) {
    return STRING_STATUS_ERR;
  }

  string->rawptr = tmp;
  return count;
}

int string_push(String* string, char ch) {
  Layout* layout;

  if (string == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;

  if (layout->needed == 0 || layout->needed == layout->size * string->length) {
    ssize_t status = string_reserve(
        string, StringStepDefault < sizeof(char) ? sizeof(char) : StringStepDefault);

    if (status == STRING_STATUS_ERR) {
      return status;
    }
  }

  string->rawptr[string->length++] = ch;
  return STRING_STATUS_OK;
}

ssize_t string_pushstr(String* string, char* cstr) {
  Layout* layout;
  size_t length;

  if (string == nullptr || cstr == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &string->layout;
  length = strlen(cstr);

  if (layout->needed > layout->size * length) {
    memcpy(string->rawptr + string->length, cstr, length);
    string->length += length;
    return length;
  }

  ssize_t status = string_reserve(string, length);

  if (status == STRING_STATUS_ERR) {
    return status;
  }

  strncat(string->rawptr, cstr, length);
  string->length += length;
  return length;
}

char* string_at(String* string, size_t index) {
  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return nullptr;

  if (string->length <= index)
    return nullptr;

  return &string->rawptr[index];
}

int string_crop(String* string) {
  Layout* current;
  char* temporary;

  if (string == nullptr || string->layout.status == LAYOUT_NULL_PTR)
    return STRING_STATUS_ERR;

  current = &string->layout;

  if (string->length == current->needed)
    return STRING_STATUS_OK;

  layout_min(current, current->needed - string->length);
  temporary = layout_realloc(current, string->rawptr);

  if (current->status != LAYOUT_NON_NULL)
    return STRING_STATUS_ERR;

  string->rawptr = temporary;
  return STRING_STATUS_OK;
}

void string_free(String* string) {
  if (string == nullptr)
    return;

  layout_dealloc(&string->layout, string->rawptr);
}
