
#ifndef SRC_LEVELS_LEVELS_H
#define SRC_LEVELS_LEVELS_H

#define GRID_SIZE 16

#include "../game/context.h"


int level_test_entry_init(struct gamectx *ctx, struct levelctx *lvl);
int level_test_adj_init(struct gamectx *ctx, struct levelctx *lvl);
int level_test_load_init(struct gamectx *ctx, struct levelctx *lvl);

void level_set_floor(struct levelctx *ctx, int x, int y);
void level_set_wall(struct levelctx *ctx, int x, int y);
#endif
