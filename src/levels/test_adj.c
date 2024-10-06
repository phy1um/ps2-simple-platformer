#include <stdlib.h>
#include <p2g/log.h>

#include "../tiles.h"
#include "levels.h"
#include "../game/context.h"

static void set_wall(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 16);
  set_tile(&ctx->collision, x, y, 1);
}

static void set_floor(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 9);
  set_tile(&ctx->collision, x, y, 1);
}

int level_test_adj_init(struct gamectx *ctx, struct levelctx *lvl) {
  lvl->update = 0;
  tile_map_init(&lvl->decoration, 80, 28, GRID_SIZE, 80*16, 16, &lvl->allocator);
  tile_map_init(&lvl->collision, 80, 28, GRID_SIZE, 80*16, 16, &lvl->allocator);
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
    for(int xx = 0; xx < 80; xx++) {
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

