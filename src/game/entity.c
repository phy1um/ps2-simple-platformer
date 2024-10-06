
#include <p2g/log.h>
#include "entity.h"
#include "camera.h"
#include "context.h"

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

