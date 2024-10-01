#include <stdio.h>
#include <stdlib.h>

#include "tga.h"

static int swizzle16(uint8_t *buffer, int width, int height) {
  const int bpp = 2;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      unsigned char tmp = buffer[(j * width + i) * bpp + 1];
      buffer[(j * width + i) * bpp + 1] =
          buffer[(j * width + i) * bpp];
      buffer[(j * width + i) * bpp] = tmp;
    }
  }
  return 0;
}

static int swizzle24(uint8_t *buffer, int width, int height) {
  const int bpp = 3;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      unsigned char tmp = buffer[(j * width + i) * bpp + 2];
      buffer[(j * width + i) * bpp + 2] =
          buffer[(j * width + i) * bpp];
      buffer[(j * width + i) * bpp] = tmp;
    }
  }
  return 0;
}

static int swizzle32(uint8_t *buffer, int width, int height) {
  const int bpp = 4;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      unsigned char tmp = buffer[(j * width + i) * bpp + 2];
      buffer[((j * width + i) * bpp) + 2] =
          buffer[(j * width + i) * bpp];
      buffer[(j * width + i) * bpp] = tmp;
      // PS2 alpha maps [0,0x80] to TGA's [0,0xFF]
      buffer[((j * width + i) * bpp) + 3] /= 2;
    }
  }
  return 0;
}



int tga_from_file(const char *file_name, struct tga_data *out) {
  FILE *f = fopen(file_name, "rb"); 
  if (!f) {
    return 1;
  }
  size_t bytes_read = fread(&out->header, 1, sizeof(struct tga_header), f);
  if (bytes_read != sizeof(struct tga_header)) {
    return 1;
  }
  // bytes per pixel
  int bpp = out->header.bps/8;
  int size = out->header.width*out->header.height*bpp;
  out->pixels_size = size;
  out->pixels = calloc(size, 1);
  bytes_read = fread(out->pixels, 1, size, f);
  if (bytes_read != size) {
    return 1;
  }

  if (out->header.bps == 32) {
    swizzle32(out->pixels, out->header.width, out->header.height);
  } else if (out->header.bps == 24) {
    swizzle24(out->pixels, out->header.width, out->header.height);
  } else if (out->header.bps == 16) {
    swizzle16(out->pixels, out->header.width, out->header.height);
  } 

  return 0;
}

