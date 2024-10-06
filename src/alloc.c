#include <string.h>

#include "alloc.h"

void *alloc_from(struct allocator *a, size_t num, size_t size) {
  size_t total = num*size;
  void *out = a->alloc(a->state, num, size);
  return memset(out, 0, total);
}

