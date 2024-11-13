#include <stdlib.h>
#include <p2g/log.h>

#include "../tga.h"
#include "../tiles.h"
#include "../draw.h"
#include "levels.h"
#include "../game/context.h"
#include "../game/entity.h"

/*
struct ldata {
  float last_x;
  int midway;
  struct ee_texture tiles;
};

static void load_textures(struct levelctx *lvl) {
  struct tga_data tga;
  int rc = tga_from_file("tiles.tga", &tga, &lvl->allocator);
  if (rc) {
    logerr("failed to load tiles.tga");
    return;
  }
  struct ldata *ld = lvl->leveldata;
  ld->tiles.pixels = tga.pixels;
  ld->tiles.size = tga.pixels_size;
  ld->tiles.width = tga.header.width;
  ld->tiles.height = tga.header.height;
  ld->tiles.vram_addr = vram_alloc(&lvl->vram, tga.pixels_size, 2048*4)/4;
  logdbg("adj tileset vram addr = %u", ld->tiles.vram_addr);
  return;
}

static int draw(struct gamectx *ctx, struct levelctx *lvl) {
  struct ldata *ld = lvl->leveldata;
  draw_upload_ee_texture(&ld->tiles);
  draw_bind_texture(&ld->tiles);
  draw_tile_map(&lvl->decoration, 8, &ld->tiles, &ctx->camera);
  return 0;
}

static int update(struct gamectx *g, struct levelctx *lvl, float dt) {
  struct ldata *ld = (struct ldata *)lvl->leveldata;
  struct entity *tgt = &g->entities[0];
  if (tgt->x > ld->midway && ld->last_x < ld->midway) {
    logdbg("load \"load\" level");
    ctx_load_level(g, level_test_load_init);
  } else if (tgt->x < ld->midway && ld->last_x > ld->midway) {
    logdbg("load \"entry\" level");
    ctx_load_level(g, level_test_entry_init);
  }   
  ld->last_x = tgt->x;
  return 0;
}

int level_test_adj_init(struct gamectx *ctx, struct levelctx *lvl) {
  logdbg("level ADJ vram head = %u", lvl->vram.head);
  lvl->active = 1;
  lvl->update = update;
  lvl->draw = draw;
  tile_map_init(&lvl->decoration, 100, 28, GRID_SIZE, 30*16, 16, &lvl->allocator);
  tile_map_init(&lvl->collision, 100, 28, GRID_SIZE, 30*16, 16, &lvl->allocator);
  for(int yy = 0; yy < lvl->decoration.height; yy++) {
    for(int xx = 0; xx < lvl->decoration.width; xx++) {
      int r = rand() % 40;
      if (r < 8) {
        set_tile(&lvl->decoration, xx, yy, r);
      }
    }
  }

  for (int yy = 0; yy < 10; yy++) {
      // set_wall(lvl, 3, 10+yy);
      // set_wall(lvl, 38, 10+yy);
    }
    for(int xx = 0; xx < 100; xx++) {
      level_set_floor(lvl, xx, 20);
    }
    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 20+xx, 17);
    }

    for(int xx = 0; xx < 4; xx++) {
      level_set_floor(lvl, 30+xx, 15);
    }


    struct ldata *ld = alloc_from(&lvl->allocator, 1, sizeof(struct ldata));
    ld->last_x = 0;
    ld->midway = lvl->collision.world_offset_x + (lvl->collision.width*GRID_SIZE/2);
    lvl->leveldata = ld;
    load_textures(lvl);

    return 0;
}

*/
