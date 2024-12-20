#include <p2g/core.h>
#include <p2g/log.h>
#include <p2g/ps2draw.h>

#include <gs_psm.h>

#include "draw.h"
#include "tga.h"
#include "tiles.h"
#include "vram.h"
#include "levels/levels.h"

#define TILE_SIZE GRID_SIZE

int draw_bind_texture(struct ee_texture *t) {
  return draw2d_bind_texture(t->vram_addr, t->width,
    t->height, GS_PSM_32);
}

int draw_upload_ee_texture(struct ee_texture *t) {
  if (t->width == 0 || t->height == 0) {
    logerr("upload texture dimensions: %d x %d", t->width, t->height);
    return 1;
  }
  draw_upload_texture(t->pixels, t->size, t->width,
      t->height, GS_PSM_32, t->vram_addr);
  return 0;
}

int put_tile(struct ee_texture *t, float tile_square, float x, float y, float w, float h, int index) {
  float uxo = tile_square / (float)t->width;
  float vyo = tile_square / (float)t->height;
  int tiles_per_row = t->width/tile_square;
  int tile_index_x = index % tiles_per_row;
  int tile_index_y = index / tiles_per_row;
  float u0 = uxo*tile_index_x;
  float v0 = vyo*tile_index_y;
  trace("drawing tile @ %f,%f with uvs: [%f,%f,%f,%f]",
      x, y, u0, v0, u0+uxo, v0+vyo);
  return draw2d_sprite(x,y,w,h,u0,v0, u0+uxo, v0+vyo);
}

int put_sprite(struct ee_texture *tex, float x, float y, float w, float h, float u0, float v0,
    float u1, float v1)
{
  return draw2d_sprite(x,y,w,h,u0,v0,u1,v1);
}

void draw_tile_map(struct tile_map *tm, float tile_square, struct ee_texture *tex, struct game_camera *cam) {
  if (!tm) {
    logerr("cannot draw NULL tile map");
    return;
  }
  if (!tm->tiles) {
    logerr("cannot draw NULL tile map data");
    return;
  }
  if (!camera_contains_area(cam, tm->world_offset_x, tm->world_offset_y, 
        tm->width*GRID_SIZE, tm->height*GRID_SIZE)) {
    return;
  }
  draw2d_set_colour(0x80, 0x80, 0x80, 0x80);
  for(int y = 0; y < tm->height; y++) {
    for(int x = 0; x < tm->width; x++) {
      unsigned char t = get_tile(tm, x, y);
      if (t != 0 && t != TILE_INVALID) {
        put_tile(tex,
            tile_square,
            tm->world_offset_x + TILE_SIZE*x - cam->position[0], 
            tm->world_offset_y + TILE_SIZE*y - cam->position[1], 
            TILE_SIZE, 
            TILE_SIZE, 
            t-1);
      }
    }
  }
}

