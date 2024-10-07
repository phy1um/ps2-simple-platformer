#include <stdlib.h>
#include <math.h>
#include <p2g/log.h>

#include "../tga.h"
#include "../tiles.h"
#include "levels.h"
#include "../draw.h"
#include "../game/context.h"

struct ldata {
  struct ee_texture tiles;
};

static void load_textures(struct levelctx *lvl) {
  struct tga_data tga;
  int rc = tga_from_file("host:tiles_test0.tga", &tga, &lvl->allocator);
  if (rc) {
    logerr("failed to load tiles_test0.tga");
    return;
  }
  struct ldata *ld = lvl->leveldata;
  ld->tiles.pixels = tga.pixels;
  ld->tiles.size = tga.pixels_size;
  ld->tiles.width = tga.header.width;
  ld->tiles.height = tga.header.height;
  ld->tiles.vram_addr = vram_alloc(&lvl->vram, tga.pixels_size, 256);
  return;
}

static int draw(struct gamectx *ctx, struct levelctx *lvl) {
  struct ldata *ld = lvl->leveldata;
  draw_upload_ee_texture(&ld->tiles);
  draw_bind_texture(&ld->tiles);
  draw_tile_map(&lvl->decoration, 16, &ld->tiles, &ctx->camera);
  return 0;
}

int level_test_load_init(struct gamectx *ctx, struct levelctx *lvl) {
  lvl->active = 1;
  lvl->update = 0;
  lvl->draw = draw;
  lvl->leveldata = alloc_from(&lvl->allocator, 1, sizeof(struct ldata));
  load_textures(lvl);
  tile_map_init(&lvl->decoration, 80, 28, GRID_SIZE, (30+100)*16, -16, &lvl->allocator);
  tile_map_init(&lvl->collision, 80, 28, GRID_SIZE, (30+100)*16, -16, &lvl->allocator);
  for(int yy = 0; yy < lvl->decoration.height; yy++) {
    for(int xx = 0; xx < lvl->decoration.width; xx++) {
      int r = rand() % 40;
      if (r < 8) {
        set_tile(&lvl->decoration, xx, yy, r);
      }
    }
  }

  for (int yy = 0; yy < 10; yy++) {
      level_set_wall(lvl, 78, 10+yy);
    }
    for(int xx = 0; xx < 80; xx++) {
      float h = sinf((xx*1.f)/20.f);
      level_set_floor(lvl, xx, ((int)(7.2*h+20)));
    }
    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 20+xx, 17);
    }

    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 30+xx, 15);
    }
    return 0;
}

