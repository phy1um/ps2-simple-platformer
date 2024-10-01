#include <p2g/core.h>
#include <p2g/log.h>
#include <p2g/ps2draw.h>

#include <gs_psm.h>

#include "draw.h"
#include "tga.h"

static uint32_t VRAM_HEAD = 0;
static uint32_t VRAM_SIZE = 4 * 1024 * 1024;
uint32_t vram_alloc(uint32_t size, uint32_t align) {
  trace("vram alloc: %lu", size);
  while (VRAM_HEAD % align != 0) {
    VRAM_HEAD += 1;
  }
  uint32_t out = VRAM_HEAD;
  if (out > VRAM_SIZE) {
    p2g_fatal("VRAM overflow");
  }
  VRAM_HEAD += size;
  return out;
}

struct ee_texture {
  void *pixels;
  size_t size;
  int width;
  int height;
  int vram_addr;
};

static struct ee_texture tex_tileset;
static int textures_in_vram = 0;

int bind_tileset() {
  return draw2d_bind_texture(tex_tileset.vram_addr, tex_tileset.width,
    tex_tileset.height, GS_PSM_32);
}

int upload_textures() {
  // assume we are in a frame 
  if (textures_in_vram) {
    return 0;
  }
  draw_upload_texture(tex_tileset.pixels, tex_tileset.size, tex_tileset.width,
      tex_tileset.height, GS_PSM_32, tex_tileset.vram_addr);
  textures_in_vram = 1;
  return 0;
}

int put_tile(float x, float y, float w, float h, int index) {
  float uxo = 8. / (float)tex_tileset.width;
  float vyo = 8. / (float)tex_tileset.height;
  int tiles_per_row = tex_tileset.width/8;
  int tile_index_x = index % tiles_per_row;
  int tile_index_y = index / tiles_per_row;
  // int tiles_per_col = tex_tileset.height/8;
  float u0 = uxo*tile_index_x;
  float v0 = vyo*tile_index_y;
  trace("drawing tile @ %f,%f with uvs: [%f,%f,%f,%f]",
      x, y, u0, v0, u0+uxo, v0+vyo);
  return draw2d_sprite(x,y,w,h,u0,v0, u0+uxo, v0+vyo);
}

int load_textures() {
  struct tga_data tga;
  int rc = tga_from_file("host:tiles.tga", &tga);
  if (rc) {
    logerr("failed to load tiles.tga");
    return rc;
  }
  tex_tileset.pixels = tga.pixels;
  tex_tileset.size = tga.pixels_size;
  tex_tileset.width = tga.header.width;
  tex_tileset.height = tga.header.height;
  tex_tileset.vram_addr = vram_alloc(tga.pixels_size, 256);
  return 0;
}

