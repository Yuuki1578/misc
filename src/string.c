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

size_t STRING_STEP_DFL = STRING_ALLOC_STEP;

String string_new(void) {
  return (String){
      .rawptr = nullptr,
      .length = 0,
      .layout = layout_new(sizeof(char), 0),
  };
}

ssize_t string_reserve(String* self, size_t count) {
  Layout* layout;
  char* tmp;

  if (self == nullptr || count == 0) {
    return STRING_STATUS_ERR;
  }

  layout = &self->layout;
  layout_add(layout, count);

  switch (layout->status) {
    case LAYOUT_NON_NULL:
      tmp = layout_realloc(layout, self->rawptr);
      break;

    default:
      tmp = layout_alloc(layout);
      break;
  }

  if (layout->status != LAYOUT_NON_NULL) {
    return STRING_STATUS_ERR;
  }

  self->rawptr = tmp;
  return count;
}

int string_push(String* self, char ch) {
  Layout* layout;

  if (self == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &self->layout;

  if (layout->needed == 0 || layout->needed == layout->size * self->length) {
    ssize_t status = string_reserve(
        self, STRING_STEP_DFL < sizeof(char) ? sizeof(char) : STRING_STEP_DFL);

    if (status == STRING_STATUS_ERR) {
      return status;
    }
  }

  self->rawptr[self->length++] = ch;
  return STRING_STATUS_OK;
}

ssize_t string_pushstr(String* self, char* cstr) {
  Layout* layout;
  size_t length;

  if (self == nullptr || cstr == nullptr) {
    return STRING_STATUS_ERR;
  }

  layout = &self->layout;
  length = strlen(cstr);

  if (layout->needed > layout->size * length) {
    memcpy(self->rawptr + self->length, cstr, length);
    self->length += length;
    return length;
  }

  ssize_t status = string_reserve(self, length);

  if (status == STRING_STATUS_ERR) {
    return status;
  }

  strncat(self->rawptr, cstr, length);
  self->length += length;
  return length;
}

char* string_at(String* self, size_t index) {
  if (self == nullptr || self->layout.status == LAYOUT_NULL_PTR)
    return nullptr;

  if (self->length <= index)
    return nullptr;

  return &self->rawptr[index];
}

int string_crop(String* self) {
  Layout* current;
  char* temporary;

  if (self == nullptr || self->layout.status == LAYOUT_NULL_PTR)
    return STRING_STATUS_ERR;

  current = &self->layout;

  if (self->length == current->needed)
    return STRING_STATUS_OK;

  layout_min(current, current->needed - self->length);
  temporary = layout_realloc(current, self->rawptr);

  if (current->status != LAYOUT_NON_NULL)
    return STRING_STATUS_ERR;

  self->rawptr = temporary;
  return STRING_STATUS_OK;
}

void string_free(String* self) {
  if (self == nullptr)
    return;

  layout_dealloc(&self->layout, self->rawptr);
}
