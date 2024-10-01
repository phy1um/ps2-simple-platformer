#ifndef PS2_SCROLLER_SRC_DRAW_H
#define PS2_SCROLLER_SRC_DRAW_H

#include <stdint.h>
#include "tiles.h"

#define SCR_WIDTH 640
#define SCR_HEIGHT 448 

uint32_t vram_alloc(uint32_t size, uint32_t align);
int bind_tileset();
int upload_textures();
int load_textures();
int put_tile(float x, float y, float w, float h, int index);
void draw_tile_map(struct tile_map *tm);

#endif
