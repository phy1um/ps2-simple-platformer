#ifndef SRC_VRAM_H
#define SRC_VRAM_H

#define VRAM_MAX (4*1024*1024)
#define VRAM_INVALID ((unsigned int)-1)

typedef unsigned int vram_addr_t;

struct vram_slice {
  vram_addr_t start;
  vram_addr_t end;
  vram_addr_t head;
};

vram_addr_t vram_alloc(struct vram_slice *v, size_t size, size_t align);
int vram_copy_slice(struct vram_slice *from, struct vram_slice *to);
int vram_slice_reset_head(struct vram_slice *v);

#endif
