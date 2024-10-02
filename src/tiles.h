#ifndef SRC_TILES_H
#define SRC_TILES_H

#define TILE_INVALID (-1)

struct tile_map {
  int width;
  int height;
  int world_offset_x;
  int world_offset_y;
  int grid_size;
  unsigned char *tiles;
};

void tile_map_init(struct tile_map *tm, int w, int h, int grid_size, int offset_x, int offset_y);
unsigned char get_tile(struct tile_map *tm, int x, int y);
void set_tile(struct tile_map *tm, int x, int y, unsigned char v);
unsigned char get_tile_world(struct tile_map *tm, float tx, float ty);

#endif
