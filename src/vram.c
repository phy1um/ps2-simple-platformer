
#include <stdlib.h>
#include <p2g/log.h>
#include "vram.h"

vram_addr_t vram_alloc(struct vram_slice *v, size_t size, size_t align) {
  if (!v) {
    logerr("alloc from NULL vram");
    return VRAM_INVALID;
  }
  if (v->head + size >= v->end) {
    logerr("vram alloc overflow"); 
    return VRAM_INVALID;
  }
  while(v->head % align != 0) {
    v->head += 1;
  }
  logdbg("vram alloc @ %u for %zu (align %zu)", v->head, size, align);
  vram_addr_t out = v->head;
  v->head += size;
  return out;
}

int vram_pad(struct vram_slice *v, size_t align) {
  if (!v) {
    logerr("pad NULL vram");
    return VRAM_INVALID;
  }
  while(v->head % align != 0) {
    v->head += 1;
  }
  return 0;
}

int vram_copy_slice(struct vram_slice *from, struct vram_slice *to) {
  if (!from || !to) {
    logerr("copy to/from NULL vram slice");
    return -1;
  }
  to->head = from->head; 
  to->start = from->start;
  to->end = from->end;
  return 0;
}

int vram_slice_reset_head(struct vram_slice *v) {
  if (!v) {
    logerr("reset head of NULL vram slice");
    return -1;
  }
  v->head = v->start;
  return 0;
}

