#include <stdlib.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include "context.h"
#include "entity.h"
#include "../tiles.h"
#include "../draw.h"

#define LEVEL_MAX 2
#define LEVEL_HEAP_SIZE (2*1024*1024)

void *level_alloc(struct levelctx *lvl, size_t num, size_t size) {
  size_t total = num*size;
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
    ctx->levels[i].collision.width = 0;
    ctx->levels[i].collision.height = 0;
    ctx->levels[i].vram.start = vram_head;
    ctx->levels[i].vram.head = vram_head;
    ctx->levels[i].vram.end = vram_head + vram_level_size;
    vram_head += vram_level_size;
    logdbg("init level slot %d: heap @ %X, size = %d", i, ctx->levels[i].heap, LEVEL_HEAP_SIZE);
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
  char t0 = get_tile_world(&ctx->levels[0].collision, x, y);
  char t1 = get_tile_world(&ctx->levels[1].collision, x, y);
  return (t0 == 0 || t0 == TILE_INVALID) && (t1 == 0 || t1 == TILE_INVALID);
}

int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h) {
  return ctx_is_free_point(ctx, x, y)
    && ctx_is_free_point(ctx, x+w, y)
    && ctx_is_free_point(ctx, x+w, y+h)
    && ctx_is_free_point(ctx, x, y+h);
}

int ctx_draw(struct gamectx *ctx) {
  trace("draw tilemaps");
  draw2d_set_colour(0x80, 0x80, 0x80, 0x80);
  if (ctx->levels[0].active) {
    if (ctx->levels[0].draw) {
      ctx->levels[0].draw(ctx, &ctx->levels[0]);
    }
  }
  if (ctx->levels[1].active) {
    if (ctx->levels[1].draw) {
      ctx->levels[1].draw(ctx, &ctx->levels[1]);
    }
  }
  
  trace("draw entities");
  entity_draw_list(ctx->entities, ENTITY_MAX, ctx);
  return 0;
}

int ctx_update(struct gamectx *ctx, float dt) {
  entity_update_list(ctx->entities, ENTITY_MAX, ctx, dt);
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (ctx->levels[i].active && ctx->levels[i].update) {
      ctx->levels[i].update(ctx, &ctx->levels[i], dt);
    }
  }
  return 0;
}

int ctx_load_level(struct gamectx *ctx, level_init_fn fn) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  struct levelctx *x = &ctx->levels[tgt_index];
  if (x->active) {
    ctx_free_level(ctx);
  }
  return fn(ctx, x);
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
  if (x->cleanup) {
    x->cleanup(ctx, x);
  }
  x->heap_head = 0;
  x->collision.width = 0;
  x->collision.height = 0;
  vram_slice_reset_head(&x->vram);
  return 0;
}

