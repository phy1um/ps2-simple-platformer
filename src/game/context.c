#include <stdlib.h>

#include <p2g/log.h>
#include "context.h"

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

