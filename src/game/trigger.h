
#ifndef SRC_GAME_TRIGGER_H
#define SRC_GAME_TRIGGER_H

#include <stdint.h>
#include <stddef.h>

#include "context.h"

enum trigger_area_kind {
  TRIGGER_NIL,
  TRIGGER_LOAD_LEVEL,
};

struct trigger_area {
  int32_t position[2];
  uint32_t size[2];
  enum trigger_area_kind kind;
  char *arg;
  size_t arg_len;
};

int trigger_collides_point(struct trigger_area *a, float x, float y);
int trigger_event(struct trigger_area *a, struct gamectx *ctx, struct levelctx *lvl);

#endif
