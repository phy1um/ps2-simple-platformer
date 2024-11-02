#include <stdio.h>
#include <stdlib.h>
#include "../wad.h"

int read_index(const char *wad_file) {
  struct wad_file wad = {0};
  if(!wad_open(wad_file, &wad)) {
    fprintf(stderr, "error: open wad %s\n", wad_file);
    return -1;
  }
  for (int i = 0; i < wad.header.index_entries; i++) {
    struct wad_index idx = wad.index[i];
    printf("%d: (hash=%X, block=%d, size=%d, ext=%d)\n", 
        i, idx.hash, idx.block_offset, idx.size, idx.ext);
  }
  return 0;
}

int read_file(const char *wad_file, const char *inner_file) {
  struct wad_file wad = {0};
  if(!wad_open(wad_file, &wad)) {
    fprintf(stderr, "error: open wad %s\n", wad_file);
    return -1;
  }
  void *data = malloc(1024*1024);
  size_t sz = wad_read_file(&wad, wad_hash_name(inner_file), data, 1024*1024);
  if (sz == 0) {
    fprintf(stderr, "error: read file %s\n", inner_file);
    return -1;
  }
  FILE *out = freopen(NULL, "wb", stdout);
  fwrite(data, sz, 1, out);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 3) {
    const char *wad_file = argv[1]; 
    const char *inner_file = argv[2]; 
    return read_file(wad_file, inner_file);
  } else if (argc == 2) {
    const char *wad_file = argv[1]; 
    return read_index(wad_file);
  } else {
    fprintf(stderr, "USAGE: %s <wad> [file]", argv[0]);
    return -1;
  }
}

