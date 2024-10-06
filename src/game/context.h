
#ifndef SRC_GAME_CONTEXT_H
#define SRC_GAME_CONTEXT_H

#include "entity.h"
#include "camera.h"
#include "../tiles.h"

#define ENTITY_MAX 120

struct gamectx;
struct levelctx;
typedef int(*level_init_fn)(struct gamectx *, struct levelctx *);
typedef int(*level_update_fn)(struct gamectx *, struct levelctx *, float dt);

struct levelctx {
  struct tile_map decoration;
  struct tile_map collision;
  level_update_fn update;
  void *heap;
  size_t heap_head;
  size_t heap_size;
  struct allocator allocator;
};

struct gamectx {
  struct entity entities[ENTITY_MAX];
  size_t entity_alloc_head;
  struct game_camera camera;
  struct levelctx levels[2];
  unsigned int active_level;
};



int ctx_init(struct gamectx *ctx);

int ctx_next_entity(struct gamectx *ctx, size_t *out);
int ctx_free_entity(struct gamectx *ctx, size_t index);
int ctx_is_free_point(struct gamectx *ctx, float x, float y);
int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h);

int ctx_draw(struct gamectx *ctx);

int ctx_load_level(struct gamectx *ctx, level_init_fn fn);
int ctx_swap_active_level(struct gamectx *ctx);

void *level_alloc(struct levelctx *lvl, size_t num, size_t size);

#endif
