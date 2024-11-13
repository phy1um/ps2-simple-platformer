#ifndef PS2_SCROLLER_SRC_DRAW_H
#define PS2_SCROLLER_SRC_DRAW_H

#include <stdint.h>
#include "tiles.h"
#include "game/camera.h"

#define SCR_WIDTH 640
#define SCR_HEIGHT 448

struct ee_texture {
  void *pixels;
  size_t size;
  int width;
  int height;
  int vram_addr;
};

int put_tile(struct ee_texture *tex, float tile_sq, float x, float y, float w, float h, int index);
void draw_tile_map(struct tile_map *tm, float tile_sq, struct ee_texture *tex, struct game_camera *camera);
int put_sprite(struct ee_texture *tex, float x, float y, float w, float h, float u0, float v0,
    float u1, float v1);

int draw_bind_texture(struct ee_texture *t);
int draw_upload_ee_texture(struct ee_texture *t);

#endif
