#include <string.h>
#include <p2g/log.h>

#include "alloc.h"

void *alloc_from(struct allocator *a, size_t num, size_t size) {
  size_t total = num*size;
  void *out = a->alloc(a->state, num, size);
#ifndef ALLOC_SKIP_CHECK
  if (!out) {
    logerr("alloc failed");
    return 0;
  }
#endif
  return memset(out, 0, total);
}

