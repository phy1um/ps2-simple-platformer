
#include <stdlib.h>
#include <p2g/log.h>
#include "tiles.h"

void tile_map_init(struct tile_map *tm, int w, int h, int offset_x, int offset_y) {
  if (!tm) {
    logerr("init NULL tilemap");
  }
  tm->width = w;
  tm->height = h;
  tm->world_offset_x = offset_x;
  tm->world_offset_y = offset_y;
  tm->tiles = calloc(1, w*h);
}

unsigned char get_tile(struct tile_map *tm, int x, int y) {
  if (x < 0 || y < 0 || x >= tm->width || y >= tm->height) {
    return TILE_INVALID;
  }
  size_t index = y*tm->width + x;
  return tm->tiles[index];
}

void set_tile(struct tile_map *tm, int x, int y, unsigned char v) {
  if (x < 0 || y < 0 || x >= tm->width || y >= tm->height) {
    return;
  }
  size_t index = y*tm->width + x;
  tm->tiles[index] = v;
}


