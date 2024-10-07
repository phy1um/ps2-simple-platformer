#include "levels.h"
#include "../tiles.h"

void level_set_wall(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 16);
  set_tile(&ctx->collision, x, y, 1);
}

void level_set_floor(struct levelctx *ctx, int x, int y) {
  set_tile(&ctx->decoration, x, y, 9);
  set_tile(&ctx->collision, x, y, 1);
}


