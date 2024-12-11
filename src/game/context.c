#include <stdlib.h>
#include <kernel.h>
#include <string.h>

#include <p2g/log.h>
#include <p2g/pad.h>
#include "context.h"
#include "entity.h"
#include "../tga.h"
#include "../tiles.h"
#include "../draw.h"

#define LEVEL_MAX 2
#define LEVEL_HEAP_SIZE (2*1024*1024)

#ifndef GLOBAL_VRAM_SIZE 
#define GLOBAL_VRAM_SIZE 224*1024
#endif
#ifndef GLOBAl_HEAP_SIZE
#define GLOBAL_HEAP_SIZE 220*1024
#endif

struct gamectx *GLOBAL_CTX = 0;

static void *global_alloc(struct gamectx *self, size_t num, size_t size) {
  size_t total = num*size;
  while (self->global_heap_head % 4 != 0) {
    self->global_heap_head += 1;
  }
  trace("global allocator: head=%zu size=%zu ptr=%p", self->global_heap_head, total, self->global_heap);
  if (self->global_heap_head + total > self->global_heap_size) {
    logerr("global heap overflow: %zu x %zu", num, size);
    return 0;
  }
  void *out = self->global_heap + self->global_heap_head;
  self->global_heap_head += total;
  return out;
}

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

static int ctx_load_statics(struct gamectx *ctx) {
  struct ee_texture *tgt = alloc_from(&ctx->global_alloc, 1, sizeof(struct ee_texture));
  struct tga_data tga;
  int rc = tga_from_file("assets/8x8.tga", &tga, &ctx->global_alloc);
  if (rc) {
    logerr("failed to load font: assets/8x8.tga");
    return 1;
  }
  tgt->pixels = tga.pixels;
  tgt->size = tga.pixels_size;
  tgt->width = tga.header.width;
  tgt->height = tga.header.height;
  size_t vr_bytes = vram_alloc(&ctx->global_vram, tga.pixels_size, 2048);
  tgt->vram_addr = vr_bytes/4;
  if (vr_bytes == VRAM_INVALID) {
    logerr("font VRAM alloc");
    return 1;
  }
  // tga and pixel data allocated forever!
  logdbg("load font @ %X", tgt->vram_addr);
  font_init(&ctx->game_font, tgt, 8, 8, 128, 64, 1.f);
  return 0;
}

int ctx_init(struct gamectx *ctx, struct vram_slice *vram) {
  GLOBAL_CTX = ctx;
  ctx->global_heap = calloc(1, GLOBAL_HEAP_SIZE);
  ctx->global_heap_head = 0;
  ctx->global_heap_size = GLOBAL_HEAP_SIZE;
  ctx->global_alloc.state = ctx;
  ctx->global_alloc.alloc = (alloc_fn) global_alloc;

  ctx->global_vram.start = vram->head;
  ctx->global_vram.head = vram->head;
  ctx->global_vram.end = ctx->global_vram.start + GLOBAL_VRAM_SIZE;

  vram_addr_t vram_size = vram->end - ctx->global_vram.end ;
  vram_addr_t vram_level_size = vram_size;
  vram_addr_t vram_head = ctx->global_vram.end;
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
    ee_sema_t sema_params = {
      .init_count = 1,
      .max_count = 1,
    };
    ctx->levels[i].sema_lock_id = CreateSema(&sema_params);
    if (ctx->levels[i].sema_lock_id == -1) {
      logerr("level %d: failed to create sema", i);
      return 1;
    }
    // vram_head += vram_level_size;
    logdbg("init level slot %d: heap @ %p, size = %d", i, ctx->levels[i].heap, LEVEL_HEAP_SIZE);
  }
  if (ctx_load_statics(ctx)) {
    logerr("ctx load statics");
    return 1;
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

static int level_poll_lock(struct gamectx *ctx, int level_id) {
  if (PollSema(ctx->levels[level_id].sema_lock_id) == -1) {
    return -1;
  }
  // we have the lock
  return 0;
}

static int level_wait_lock(struct gamectx *ctx, int level_id) {
  WaitSema(ctx->levels[level_id].sema_lock_id);
  return 0;
}



static int level_signal_lock(struct gamectx *ctx, int level_id) {
  // TODO: what does the RV of this function mean????
  SignalSema(ctx->levels[level_id].sema_lock_id); 
  return 0;
}

// WARNING: only call if we have the lock for this level
static int __locked_level_point_free(struct levelctx *lvl, float x, float y) {
  if (!lvl->test_point) {
    return 1;
  }
  int hit = lvl->test_point(lvl, x, y);
  return !hit;
}


int ctx_is_free_point(struct gamectx *ctx, float x, float y) {
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (level_poll_lock(ctx, i) == -1) {
      continue;
    }
    // we have the lock
    int res = __locked_level_point_free(&ctx->levels[i], x, y);
    level_signal_lock(ctx, i);
    // released the lock
    if (!res) {
      return 0;
    }
  }
  return 1;
}

int ctx_is_free_box(struct gamectx *ctx, float x, float y, float w, float h) {
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (level_poll_lock(ctx, i) == -1) {
      continue;
    }
    struct levelctx *lvl = &ctx->levels[i];
    int res = __locked_level_point_free(lvl, x, y)
      && __locked_level_point_free(lvl, x+w, y)
      && __locked_level_point_free(lvl, x+w, y+h)
      && __locked_level_point_free(lvl, x, y+h);
    level_signal_lock(ctx, i);
    if (!res) {
      return 0;
    }
  }
  return 1;
}

int ctx_draw(struct gamectx *ctx) {
  for(int i = 0; i < LEVEL_MAX; i++) {
    if (level_poll_lock(ctx, i) == -1) {
      continue;
    }
    if (ctx->levels[i].active 
        && camera_contains_bounds(&ctx->camera, 
          ctx->levels[i].bounds[0],
          ctx->levels[i].bounds[1],
          ctx->levels[i].bounds[2],
          ctx->levels[i].bounds[3])) {
      if (ctx->levels[i].draw) {
        ctx->levels[i].draw(ctx, &ctx->levels[i]);
      }
    }
    level_signal_lock(ctx, i);
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
    if (level_poll_lock(ctx, i) == -1) {
      continue;
    }
    if (ctx->levels[i].active && ctx->levels[i].update) {
      ctx->levels[i].update(ctx, &ctx->levels[i], dt);
      struct entity *player = ctx_get_player(ctx);
      if (ctx_point_in_level(&ctx->levels[i], player->x, player->y)) {
        ctx->active_level = i;
      }
    }
    level_signal_lock(ctx, i);
  }
  return 0;
}

int ctx_reload(struct gamectx *ctx) {
  for (int i = 0; i < LEVEL_MAX; i++) {
    if (level_poll_lock(ctx, i) == -1) {
      continue;
    }
    if (!ctx->levels[i].reload) {
      continue;
    }
    if (ctx->levels[i].reload(ctx, &ctx->levels[i])) {
      logerr("reload level: %d", i); 
      level_signal_lock(ctx, i);
      return 1;
    } else {
      level_signal_lock(ctx, i);
    }
  }
  return 0;
}

int ctx_load_level(struct gamectx *ctx, level_init_fn fn, const char *arg) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  level_wait_lock(ctx, tgt_index);
  struct levelctx *x = &ctx->levels[tgt_index];
  if (x->loaded_name != 0 && strcmp(x->loaded_name, arg) == 0) {
    info("skip load: level already loaded in slot %d", tgt_index);
    level_signal_lock(ctx, tgt_index);
    return 0;
  }

  if (x->active) {
    __locked_ctx_free_level(ctx, tgt_index);
  }
  int rc = fn(ctx, x, arg);
  level_signal_lock(ctx, tgt_index);
  return rc;
}

int ctx_swap_active_level(struct gamectx *ctx) {
  unsigned int tgt_index = (ctx->active_level + 1) % LEVEL_MAX;
  ctx->active_level = tgt_index;
  return 0;
}

int __locked_ctx_free_level(struct gamectx *ctx, int tgt_index) {
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

int ctx_print_stats(struct gamectx *ctx) {
  const char level_active_marker[] = "*";
  const char level_inactive_marker[] = "";

  float global_heap_pc = ((float)ctx->global_heap_head) / ((float)ctx->global_heap_size);
  float global_vram_pc = ((float)(ctx->global_vram.head - ctx->global_vram.start)/((float)ctx->global_vram.end - ctx->global_vram.start));
  logdbg("stat(global): [heap %.02f%% (%zu / %zu)] [vram %.02f%% (%zu / %zu)]", 
    global_heap_pc*100, ctx->global_heap_head, ctx->global_heap_size,    
    global_vram_pc*100, ctx->global_vram.head, ctx->global_vram.end
  );
  for (int i = 0; i < LEVEL_MAX; i++) {
    struct levelctx *lvl = &ctx->levels[i];
    float heap_pc = ((float)lvl->heap_head) / ((float)lvl->heap_size);
    float vram_pc = ((float)(lvl->vram.head - lvl->vram.start)/((float)lvl->vram.end - lvl->vram.start));
    const char *marker = ctx->active_level == i ? level_active_marker : level_inactive_marker;
    logdbg("stat(%d)%s: [heap %.02f%% (%zu / %zu)] [vram %.02f%% (%zu / %zu)]", 
        i,
        marker,
        heap_pc*100, lvl->heap_head, lvl->heap_size,
        vram_pc*100, lvl->vram.head, lvl->vram.end);
  }
  return 0;
}

