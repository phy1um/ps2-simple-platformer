
#include <stdint.h>

#ifndef SRC_GAME_TGA_H
#define SRC_GAME_TGA_H

struct __attribute__((__packed__)) tga_header {
  uint8_t idlen;
  uint8_t colMapType;
  uint8_t imgType;

  uint16_t firstColEntryIndex;
  uint16_t colMapLength;
  uint8_t colMapBps;

  uint16_t xorigin;
  uint16_t yorigin;
  uint16_t width;
  uint16_t height;
  uint8_t bps;
  uint8_t descriptor;
};

struct tga_data {
  struct tga_header header;
  uint8_t *pixels;
  size_t pixels_size;
};


int tga_from_file(const char *file_name, struct tga_data *out);

#endif
