
#ifndef SRC_GAME_CONTEXT_H
#define SRC_GAME_CONTEXT_H

#include "entity.h"

#define ENTITY_MAX 120

struct gamectx {
  struct entity entities[ENTITY_MAX];
  size_t entity_alloc_head;
};

int ctx_next_entity(struct gamectx *ctx, size_t *out);
int ctx_free_entity(struct gamectx *ctx, size_t index);

#endif
