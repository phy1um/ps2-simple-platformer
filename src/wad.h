#ifndef SRC_WAD_H
#define SRC_WAD_H

#include <stdint.h>
#include <stdio.h>

struct wad_index {
  uint32_t hash;
  uint32_t block_offset;
  uint32_t size;
  uint32_t ext;
};

struct wad_header {
  uint32_t id;
  uint32_t version;
  uint32_t index_entries;
  uint32_t index_offset;
  uint32_t block_size;
};

struct wad_file {
  struct wad_index *index;
  struct wad_header header;
  FILE *f;
};

uint32_t wad_hash_name(const unsigned char *fname);
int wad_open(const char *filename, struct wad_file *tgt);
size_t wad_read_file(struct wad_file *w, uint32_t file_name_hash,
    void *target_buffer, size_t target_buffer_size);
size_t wad_read_file_part(struct wad_file *w, uint32_t file_name_hash,
    void *target_buffer, size_t target_buffer_size,
    size_t file_start, size_t file_end);
size_t wad_file_size(struct wad_file *w, uint32_t file_name_hash);

#endif
