#include "io.h"
#include "wad.h"

static struct wad_file WAD = {0};

int io_init_wad(const char *wad_path) {
  return wad_open(wad_path, &WAD);
}

size_t io_get_file_size(const char *file_name) {
  return wad_file_size(&WAD, wad_hash_name((unsigned char *)file_name));
}

size_t io_read_file(const char *file_name, void *target_buffer,
    size_t target_buffer_size) {
  return wad_read_file(&WAD, wad_hash_name((unsigned char *)file_name), target_buffer, target_buffer_size); 
}

size_t io_read_file_part(const char *file_name, void *target_buffer,
    size_t target_buffer_size, size_t start, size_t end) {
  return wad_read_file_part(&WAD, wad_hash_name((unsigned char *)file_name), target_buffer, target_buffer_size,
      start, end); 
}

