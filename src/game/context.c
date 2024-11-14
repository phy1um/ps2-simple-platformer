#include <stdlib.h>

#include <p2g/log.h>
#include <p2g/pad.h>
#include "context.h"
#include "entity.h"
#include "../tiles.h"
#include "../draw.h"

#define LEVEL_MAX 2
#define LEVEL_HEAP_SIZE (2*1024*1024)

static int ctx_point_in_level(struct levelctx *lvl, float x, float y) {
  return (
      x >= lvl->bounds[0]
      && y >= lvl->bounds[1]
      && x <= lvl->bounds[2]
      && y <= lvl->bounds[3]
    );
}

void *level_alloc(struct levelctx *lvl, size_t num, size_t size) {
  size_t total = num*size;
  while (lvl->heap_head % 4 != 0) {
    lvl->heap_head += 1;
  }
  trace("level bump allocator: head=%zu size=%zu ptr=%p", lvl->heap_head, total, lvl->heap);
  if (lvl->heap_head + total > lvl->heap_size) {
    logerr("level heap overflow: %zu x %zu", num, size);
    return 0;
  }
  void *out = lvl->heap + lvl->heap_head;
  lvl->heap_head += total;
  return out;
}


int ctx_init(struct gamectx *ctx, struct vram_slice *vram) {
  vram_addr_t vram_size = vram->end - vram->head;
  vram_addr_t vram_level_size = vram_size / LEVEL_MAX;
  vram_addr_t vram_head = vram->head;
  for (int i = 0; i < LEVEL_MAX; i++) {
    trace("init level %i", i);
    void *heap = calloc(1, LEVEL_HEAP_SIZE);
    if (!heap) {
      logerr("ctx init: allocating level (%d) heap failed", i);
      return -1;
    }
    ctx->levels[i].heap = heap; 
    ctx->levels[i].heap_head = 0;
    ctx->levels[i].heap_size = LEVEL_HEAP_SIZE;
    ctx->levels[i].allocator.state = &ctx->levels[i];
    ctx->levels[i].allocator.alloc = (alloc_fn) level_alloc;
    ctx->levels[i].vram.start = vram_head;
    ctx->levels[i].vram.head = vram_head;
    ctx->levels[i].vram.end = vram_head + vram_level_size;
    vram_head += vram_level_size;
    logdbg("init level slot %d: heap @ %p, size = %d", i, ctx->levels[i].heap, LEVEL_HEAP_SIZE);
  }
  return 0;
}

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
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (!ctx->levels[i].test_point) {
      continue;
    }
    int hit = ctx->levels[i].test_point(&ctx->levels[i], x, y);
    if (hit) {
      return 0;
    }
  }
  return 1;
}

int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h) {
  return ctx_is_free_point(ctx, x, y)
    && ctx_is_free_point(ctx, x+w, y)
    && ctx_is_free_point(ctx, x+w, y+h)
    && ctx_is_free_point(ctx, x, y+h);
}

int ctx_draw(struct gamectx *ctx) {
  trace("draw tilemaps");
  if (ctx->levels[0].active 
      && camera_contains_bounds(&ctx->camera, 
        ctx->levels[0].bounds[0],
        ctx->levels[0].bounds[1],
        ctx->levels[0].bounds[2],
        ctx->levels[0].bounds[3])) {
    if (ctx->levels[0].draw) {
      ctx->levels[0].draw(ctx, &ctx->levels[0]);
    }
  }
  if (ctx->levels[1].active
      && camera_contains_bounds(&ctx->camera, 
        ctx->levels[1].bounds[0],
        ctx->levels[1].bounds[1],
        ctx->levels[1].bounds[2],
        ctx->levels[1].bounds[3])) {
    if (ctx->levels[1].draw) {
      ctx->levels[1].draw(ctx, &ctx->levels[1]);
    }
  }
  
  trace("draw entities");
  entity_draw_list(ctx->entities, ENTITY_MAX, ctx);
  return 0;
}

int ctx_update(struct gamectx *ctx, float dt) {
  // TODO: update this when there are more entities..
  if (button_pressed(BUTTON_SELECT)) {
    logdbg("active area bounds: [%f, %f, %f, %f]", 
        ctx->levels[ctx->active_level].bounds[0],
        ctx->levels[ctx->active_level].bounds[1],
        ctx->levels[ctx->active_level].bounds[2],
        ctx->levels[ctx->active_level].bounds[3]);
    logdbg("camera bounds: [%f, %f, %f, %f]",
        ctx->camera.position[0],
        ctx->camera.position[1],
        ctx->camera.position[0] + ctx->camera.bounds[0],
        ctx->camera.position[1] + ctx->camera.bounds[1]);
  }

  ctx->player_index = 0;
  entity_update_list(ctx->entities, ENTITY_MAX, ctx, dt);
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (ctx->levels[i].active && ctx->levels[i].update) {
      ctx->levels[i].update(ctx, &ctx->levels[i], dt);
      struct entity *player = ctx_get_player(ctx);
      if (ctx_point_in_level(&ctx->levels[i], player->x, player->y)) {
        ctx->active_level = i;
      }
    }
  }
  return 0;
}

int ctx_reload(struct gamectx *ctx) {
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (!ctx->levels[i].reload) {
      continue;
    }
    if (ctx->levels[i].reload(ctx, &ctx->levels[i])) {
      logerr("reload level: %d", i); 
      return 1;
    }
  }
  return 0;
}

int ctx_load_level(struct gamectx *ctx, level_init_fn fn, const char *arg) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  struct levelctx *x = &ctx->levels[tgt_index];
  if (x->active) {
    ctx_free_level(ctx);
  }
  return fn(ctx, x, arg);
}

int ctx_swap_active_level(struct gamectx *ctx) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  ctx->active_level = tgt_index;
  return 0;
}

int ctx_free_level(struct gamectx *ctx) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  struct levelctx *x = &ctx->levels[tgt_index];
  x->active = 0;
  x->draw = 0;
  x->update = 0;
  x->test_point = 0;
  x->reload = 0;
  if (x->cleanup) {
    x->cleanup(ctx, x);
    x->cleanup = 0;
  }
  x->heap_head = 0;
  vram_slice_reset_head(&x->vram);
  return 0;
}

struct entity *ctx_get_player(struct gamectx *ctx) {
  if (ctx->player_index >= ENTITY_MAX) {
    logerr("get player: player index: %zu", ctx->player_index);
    return 0;
  }
  return &ctx->entities[ctx->player_index];
}

struct levelctx * ctx_get_active_level(struct gamectx *ctx) {
  return &ctx->levels[ctx->active_level];
}
struct levelctx * ctx_get_inactive_level(struct gamectx *ctx) {
  return &ctx->levels[(ctx->active_level+1)%LEVEL_MAX];
}

