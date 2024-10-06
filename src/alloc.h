#ifndef SRC_ALLOC_H
#define SRC_ALLOC_H

#include <stddef.h>

typedef void*(*alloc_fn)(void *self, size_t num, size_t size);

struct allocator {
  void *state;
  alloc_fn alloc;
};

void *alloc_from(struct allocator *a, size_t num, size_t size);

#endif
