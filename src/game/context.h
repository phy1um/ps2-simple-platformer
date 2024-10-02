
#ifndef SRC_GAME_CONTEXT_H
#define SRC_GAME_CONTEXT_H

#include "entity.h"
#include "../tiles.h"

#define ENTITY_MAX 120

struct gamectx {
  struct entity entities[ENTITY_MAX];
  size_t entity_alloc_head;
  struct tile_map decoration;
  struct tile_map collision;
};

int ctx_next_entity(struct gamectx *ctx, size_t *out);
int ctx_free_entity(struct gamectx *ctx, size_t index);
int ctx_is_free_point(struct gamectx *ctx, float x, float y);
int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h);

#endif
