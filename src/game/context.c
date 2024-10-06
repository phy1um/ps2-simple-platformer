#include <stdlib.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include "context.h"
#include "entity.h"
#include "tiles.h"
#include "../draw.h"

int ctx_next_entity(struct gamectx *ctx, size_t *out) {
  for (size_t i = 0; i < ENTITY_MAX; i++) {
    struct entity *e = &(ctx->entities[i]);
    if (!e->active) {
      *out = i;
      return 0;
    }
  }
  logerr("no entity slot free");
  return 1;
}

int ctx_free_entity(struct gamectx *ctx, size_t index) {
  if (index >= ENTITY_MAX) {
    logerr("free entity out of bounds: %zu (max = %zu)", index, ENTITY_MAX);
    return 1;
  }
  struct entity *e = &(ctx->entities[index]);
  e->active = 0;
  if (e->data) {
    free(e->data);
    e->data = 0;
  }
  return 0;
}

int ctx_is_free_point(struct gamectx *ctx, float x, float y) {
  return !get_tile_world(&ctx->collision, x, y);
}

int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h) {
  return ctx_is_free_point(ctx, x, y)
    && ctx_is_free_point(ctx, x+w, y)
    && ctx_is_free_point(ctx, x+w, y+h)
    && ctx_is_free_point(ctx, x, y+h);
}

int ctx_draw(struct gamectx *ctx) {
  trace("bind tileset");
  bind_tileset();
  trace("draw tilemap");
  draw2d_set_colour(0x80, 0x80, 0x80, 0x80);
  draw_tile_map(&ctx->decoration, &ctx->camera);
  trace("draw entities");
  entity_draw_list(ctx->entities, ENTITY_MAX, ctx);
  return 0;
}

