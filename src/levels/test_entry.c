#include <stdlib.h>
#include <p2g/log.h>

#include "../tga.h"
#include "../tiles.h"
#include "../draw.h"
#include "levels.h"
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
  ld->tiles.vram_addr = vram_alloc(&lvl->vram, tga.pixels_size, 2048)/4;
  return;
}

static int draw(struct gamectx *ctx, struct levelctx *lvl) {
  struct ldata *ld = lvl->leveldata;
  draw_upload_ee_texture(&ld->tiles);
  draw_bind_texture(&ld->tiles);
  draw_tile_map(&lvl->decoration, 16, &ld->tiles, &ctx->camera);
  return 0;
}

int level_test_entry_init(struct gamectx *ctx, struct levelctx *lvl) {
  logdbg("level ENTRY vram head = %u", lvl->vram.head);
  lvl->active = 1;
  lvl->update = 0;
  lvl->draw = draw;

  lvl->leveldata = alloc_from(&lvl->allocator, 1, sizeof(struct ldata));
  load_textures(lvl);

  tile_map_init(&lvl->decoration, 30, 28, GRID_SIZE, 0, 0, &lvl->allocator);
  tile_map_init(&lvl->collision, 30, 28, GRID_SIZE, 0, 0, &lvl->allocator);

  for (int yy = 0; yy < 10; yy++) {
      level_set_wall(lvl, 3, 10+yy);
      // set_wall(lvl, 38, 10+yy);
    }
    for(int xx = 0; xx < lvl->collision.width; xx++) {
      level_set_floor(lvl, xx, 20);
    }
    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 20+xx, 17);
    }

    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 30+xx, 15);
    }
    return 0;
}

