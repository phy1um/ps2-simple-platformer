#ifndef PS2_SCROLLER_SRC_DRAW_H
#define PS2_SCROLLER_SRC_DRAW_H

#include <stdint.h>
#include "tiles.h"
#include "game/camera.h"

#define SCR_WIDTH 640
#define SCR_HEIGHT 448 

int bind_tileset();
int upload_textures();
int load_textures();
int put_tile(float x, float y, float w, float h, int index);
void draw_tile_map(struct tile_map *tm, struct game_camera *camera);

#endif
