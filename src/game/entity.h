#ifndef SRC_GAME_ENTITY_H
#define SRC_GAME_ENTITY_H

#include <stddef.h>

struct gamectx;

struct entity {
  int active;
  float x;
  float y;
  float w;
  float h;
  int(*draw)(struct entity *self, struct gamectx *ctx);
  int(*update)(struct entity *self, struct gamectx *ctx, float dt);
  void *data;
};

int entity_update_list(struct entity *list, size_t list_size, struct gamectx *ctx, float dt);
int entity_draw_list(struct entity *list, size_t list_size, struct gamectx *ctx);

#endif
