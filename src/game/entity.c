
#include <p2g/log.h>
#include "entity.h"
#include "camera.h"
#include "context.h"

static struct entity_id ENTITY_INVALID = {
  .index = -1,
};

struct entity_id entity_spawn(
    struct entity *list, 
    size_t list_size, 
    struct entity_class *cls, 
    struct allocator *a,
    float pos[2], 
    void *arg
) {
  if (!list) {
    logerr("spawn in NULL entity list");
    return ENTITY_INVALID;
  }
  if (!cls) {
    logerr("spawn entity NULL class");
    return ENTITY_INVALID;
  }
  if (!a) {
    logerr("spawn entity NULL allocator");
    return ENTITY_INVALID;
  }
  for (size_t i = 0; i < list_size; i++) {
    struct entity *e = &list[i];
    if (!e->active) {
      logdbg("found entity slot: %s", cls->identifier);
      e->data = alloc_from(a, 1, cls->data_size);
      if (!e->data) {
        logerr("alloc entity data: %s", cls->identifier);
        return ENTITY_INVALID;
      }
      logdbg("allocated entity data..");
      e->x = pos[0];
      e->y = pos[1];
      e->update = cls->update;
      e->draw = cls->draw;
      if (cls->init) {
        logdbg("class init is nonnull, calling");
        if (cls->init(e, arg)) {
          logerr("spawn entity %s", cls->identifier);
          return ENTITY_INVALID;
        }
      }
      e->active = 1;
      struct entity_id rv = {
        .index = i,
      };
      return rv;
    }
  }
  logerr("spawn entity: no space");
  return ENTITY_INVALID;
}

int entity_update_list(struct entity *list, size_t list_size, struct gamectx *ctx, float dt) {
  for (size_t i = 0; i < list_size; i++) {
    struct entity *e = &list[i];
    if (e->active) {
      int rc = e->update(e, ctx, dt);
      if (rc) {
        logerr("update entity %zu", i);
        return rc;
      }
    }
  }
  return 0;
}

int entity_draw_list(struct entity *list, size_t list_size, struct gamectx *ctx) {
  for (size_t i = 0; i < list_size; i++) {
    struct entity *e = &list[i];
    if (e->active) {
      float pos[2];
      camera_transform(&ctx->camera, e->x, e->y, pos);
      int rc = e->draw(e, pos[0], pos[1], ctx);
      if (rc) {
        logerr("draw entity %zu", i);
        return rc;
      }
    }
  }
  return 0;
}

