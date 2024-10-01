#ifndef SRC_TILES_H
#define SRC_TILES_H

#define TILE_INVALID (-1)

struct tile_map {
  int width;
  int height;
  int world_offset_x;
  int world_offset_y;
  unsigned char *tiles;
};

void tile_map_init(struct tile_map *tm, int w, int h, int offset_x, int offset_y);
unsigned char get_tile(struct tile_map *tm, int x, int y);
void set_tile(struct tile_map *tm, int x, int y, unsigned char v);

#endif
