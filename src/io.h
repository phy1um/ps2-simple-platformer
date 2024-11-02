#ifndef SRC_IO_H
#define SRC_IO_H

#include <stdlib.h>

size_t io_read_file(const char *file_name, void *target_buffer,
    size_t target_buffer_size);
size_t io_read_file_part(const char *file_name, void *target_buffer,
    size_t target_buffer_size, size_t start, size_t end);
size_t io_get_file_size(const char *file_name);
int io_init_wad(const char *wad_path);

#endif
