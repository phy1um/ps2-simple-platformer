#include <stdlib.h>
#include <p2g/log.h>

#include "../tiles.h"
#include "levels.h"
#include "../game/context.h"
#include "../game/entity.h"

struct ldata {
  float last_x;
  int midway;
};

static void set_wall(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 16);
  set_tile(&ctx->collision, x, y, 1);
}

static void set_floor(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 9);
  set_tile(&ctx->collision, x, y, 1);
}

static int update(struct gamectx *g, struct levelctx *lvl, float dt) {
  struct ldata *ld = (struct ldata *)lvl->heap;
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
  lvl->active = 1;
  lvl->update = update;
  struct ldata *ld = alloc_from(&lvl->allocator, 1, sizeof(struct ldata));
  ld->last_x = 0;
  tile_map_init(&lvl->decoration, 100, 28, GRID_SIZE, 30*16, 16, &lvl->allocator);
  tile_map_init(&lvl->collision, 100, 28, GRID_SIZE, 30*16, 16, &lvl->allocator);
  ld->midway = lvl->collision.world_offset_x + (lvl->collision.width*GRID_SIZE/2);
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
      set_floor(lvl, xx, 20);
    }
    for(int xx = 0; xx < 4; xx++) {
      set_floor(lvl, 20+xx, 17);
    }

    for(int xx = 0; xx < 4; xx++) {
      set_floor(lvl, 30+xx, 15);
    }
    return 0;
}

