#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <p2g/log.h>

#include "tga.h"
#include "io.h"
#include "alloc.h"

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



int tga_from_file(const char *file_name, struct tga_data *out, struct allocator *a) {
  size_t bytes_read = io_read_file_part(file_name, &out->header, sizeof(struct tga_header), 0,
      sizeof(struct tga_header));
  if (bytes_read != sizeof(struct tga_header)) {
    return 1;
  }
  if (out->header.width == 0 || out->header.height == 0) {
    logerr("invalid tga: width=%d height=%d", out->header.width, out->header.height);
    return 1;
  }
  // bytes per pixel
  int bpp = out->header.bps/8;
  int size = out->header.width*out->header.height*bpp;
  out->pixels_size = size;
  out->pixels = alloc_from(a, size, 1);
  if (!out->pixels) {
    logerr("tga pixels alloc (size = %d)", size);
    return 1;
  }

  int is_rle = (out->header.imgType & 0x8) > 0;

  if (!is_rle) {
    bytes_read = io_read_file_part(file_name, out->pixels, size, sizeof(struct tga_header), 
        size+sizeof(struct tga_header));
    if (bytes_read != size) {
      return 1;
    }
  } else {
    logdbg("loading %s as RLE", file_name);
    size_t read_head = sizeof(struct tga_header);
    int pixel_count = out->header.width*out->header.height;
    size_t pixel_head = 0;
    while(pixel_count > 0) {
      uint8_t rep_count_field; 
      if (io_read_file_part(file_name, &rep_count_field, 1, read_head, read_head+1) != 1) {
        logerr("read tga: get repetition count field");
        return 1;
      }
      read_head += 1;
      int is_run_packet = rep_count_field&0x80;
      int rpt = (rep_count_field&0x7F)+1;
      if (is_run_packet) {
        unsigned char colbuf[bpp];
        if (io_read_file_part(file_name, &colbuf, bpp, read_head, read_head+bpp) != bpp) {
          logerr("read tga: get repeated pixel value");
          return 1;
        }
        read_head += bpp;
        for (int i = 0; i < rpt; i++) {
          memcpy(out->pixels+(pixel_head+i)*bpp, colbuf, bpp);
        }
      } else {
        for (int i = 0; i < rpt; i++) {
          unsigned char colbuf[bpp];
          if (io_read_file_part(file_name, &colbuf, bpp, read_head, read_head+bpp) != bpp) {
            logerr("read tga: get repeated pixel value");
            return 1;
          }
          read_head += bpp;
          memcpy(out->pixels+(pixel_head+i)*bpp, colbuf, bpp);
        }
      }
      pixel_head += rpt;
      pixel_count -= rpt;
    }
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


