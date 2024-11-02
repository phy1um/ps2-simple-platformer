#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <p2g/log.h>

#include "wad.h"

// private functions
struct wad_index * get_index_entry(struct wad_file *w, uint32_t name_hash) {
  for (int i = 0; i < w->header.index_entries; i++) {
    trace("test index %X vs %X", name_hash, w->index[i].hash);
    if (w->index[i].hash == name_hash) {
      return &w->index[i];
    }
  }
  return 0;
}

// public api

// djb2 from http://www.cse.yorku.ca/~oz/hash.html
uint32_t wad_hash_name(const unsigned char *fname) {
  const char *fs = fname;
  uint32_t hash = 5381;
  unsigned char c;
  while (c = *fname++) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  trace("hash name %s -> %X", fs, hash);
  return hash;
}

int wad_open(const char *filename, struct wad_file *tgt) {
  FILE *f = fopen(filename, "rb"); 
  if (!f) {
    logerr("open \"%s\"", filename);
    return 0;
  }
  long rb = fread(&tgt->header, 1, sizeof(struct wad_header), f);
  if (rb != sizeof(struct wad_header)) {
    logerr("open wad header \"%s\": read wrong # bytes: %ld", filename, rb);
    return 0;
  }
  // TODO: check id/version 
  tgt->index = malloc(sizeof(struct wad_index) * tgt->header.index_entries);
  if (fseek(f, tgt->header.index_offset*tgt->header.block_size, SEEK_SET)) {
    logerr("open wad header \"%s\": seek to index block %d (%d)", filename, tgt->header.index_offset,
        tgt->header.index_offset*tgt->header.block_size);
    return 0;
  }
  long ri = fread(tgt->index, sizeof(struct wad_index), tgt->header.index_entries, f);
  if (ri != tgt->header.index_entries) {
    logerr("open wad header \"%s\": read index size (got %d expected %zu)", filename, ri, tgt->header.index_entries);
    //return 0;
  }
  tgt->f = f;
  return 1;
}

size_t wad_read_file_part(struct wad_file *w, uint32_t file_name_hash,
    void *target_buffer, size_t target_buffer_size,
    size_t file_start, size_t file_end) 
{
  if (!w) {
    logerr("null wad");
    return 0;
  }
  struct wad_index *index = get_index_entry(w, file_name_hash);
  if (!index) {
    logerr("file not found: %d", file_name_hash);
    return 0;
  }
  if (file_end == 0) {
    file_end = index->size;
  }
  size_t read_size = file_end - file_start;
  if (read_size > target_buffer_size) {
    logerr("read file %d: target too small (%zu / %zu)", file_name_hash, read_size, target_buffer_size);
    return 0;
  }
  if (fseek(w->f, index->block_offset*w->header.block_size + file_start, SEEK_SET)) {
    logerr("failed to seek %d to block offset %d", file_name_hash, index->block_offset);
    return 0;
  }
  if (fread(target_buffer, read_size, 1, w->f) != 1) {
    logerr("failed to read enough bytes %X", file_name_hash); 
    return 0;
  }
  return read_size;

}


size_t wad_read_file(struct wad_file *w, uint32_t file_name_hash,
    void *target_buffer, size_t target_buffer_size)
{
  return wad_read_file_part(w, file_name_hash, target_buffer, target_buffer_size, 0, 0);
}

size_t wad_file_size(struct wad_file *w, uint32_t file_name_hash) {
   if (!w) {
    logerr("null wad");
    return 0;
  }
  struct wad_index *index = get_index_entry(w, file_name_hash);
  if (!index) {
    logerr("file not found: %d", file_name_hash);
    return 0;
  }
  return index->size;
}

