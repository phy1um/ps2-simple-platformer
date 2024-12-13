#ifndef SRC_GAME_ENTITY_H
#define SRC_GAME_ENTITY_H

#include "../alloc.h"
#include <stddef.h>

struct entity_id {
  size_t index;
  size_t generation;
};

struct gamectx;

struct entity {
  int active;
  float x;
  float y;
  int(*draw)(struct entity *self, float sx, float sy, struct gamectx *ctx);
  int(*update)(struct entity *self, struct gamectx *ctx, float dt);
  void *data;
};

struct entity_class {
  char identifier[10];
  int(*init)(struct entity *self, void *arg);
  int(*draw)(struct entity *self, float sx, float sy, struct gamectx *ctx);
  int(*update)(struct entity *self, struct gamectx *ctx, float dt);
  size_t data_size;
};

struct entity_id entity_spawn(
    struct entity *list, 
    size_t list_size, 
    struct entity_class *cls, 
    struct allocator *a,
    float pos[2], 
    void *arg);

int entity_update_list(struct entity *list, size_t list_size, struct gamectx *ctx, float dt);
int entity_draw_list(struct entity *list, size_t list_size, struct gamectx *ctx);

#endif
