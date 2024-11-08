#include <stdio.h>
#include <stdlib.h>
#include "../wad.h"

int log_output_level = 0;

// ./packwad file1.tga file2.txt file3.foo
int main(int argc, char *argv[]) {
  int file_count = argc-1;
  int wad_total_file_size = 0;
  int block_size = 1024; // TODO: make this an arg
  size_t buffer_size = 1024*1024;
  char *buffer = malloc(buffer_size);
  size_t buffer_head = 0;

  // 1. write a header
  struct wad_header *head = (struct wad_header *) buffer;
  head->version = 1;
  head->id = 0xAABB;
  head->index_entries = file_count;
  head->index_offset = 1;
  head->block_size = block_size;
  // ASSUMPTION: header < block_size
  buffer_head = block_size;
  size_t index_head = buffer_head;
  // 2. pre-alloc index
  struct wad_index *index = (struct wad_index *) (buffer+buffer_head);
  fprintf(stderr, "wad index offset = %X (%p %p)\n", ((char*)index) - buffer, buffer+buffer_head, index);
  size_t index_size = sizeof(struct wad_index)*head->index_entries;
  buffer_head += index_size;
  // 3. put files in buffer
  for (int i = 1; i < argc; i++) {
    while(buffer_head % block_size != 0) {
      buffer_head += 1;
    }
    const char *fname = argv[i]; 
    fprintf(stderr, "packing file name: %s\n", fname);
    FILE *tgt = fopen(fname, "rb");
    fseek(tgt, 0, SEEK_END);
    long size = ftell(tgt);
    fseek(tgt, 0, SEEK_SET);
    // TODO: grow if too big
    if (buffer_head+size > buffer_size) {
      buffer_size = (buffer_head+size)*2;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        fprintf(stderr, "failed to grow buffer\n");
        return -1;
      }
      index = (struct wad_index *) (buffer+index_head);
    }
    fread(buffer+buffer_head, 1, size, tgt);
    fprintf(stderr, "index addr = %X\n", (((char*)index) - buffer)); 
    index[i-1].hash = wad_hash_name(fname);
    index[i-1].block_offset = buffer_head / block_size;
    index[i-1].size = size;
    index[i-1].ext = 0;
    buffer_head += size;
  }

  FILE *out = freopen(NULL, "wb", stdout);
  fwrite(buffer, 1, buffer_head, out);

  free(buffer);
}
