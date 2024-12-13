#ifndef SRC_GAME_RESOURCE_H
#define SRC_GAME_RESOURCE_H

#include "../draw.h"

struct image_resource_entry {
  int active;
  uint32_t name_hash;
  struct ee_texture *tex;
};

#define IMAGE_MAX 10

struct resources {
  struct image_resource_entry images[IMAGE_MAX];
};

int resource_insert_image(struct resources *res, const char *name, struct ee_texture *tex);
struct ee_texture *resource_get_image(struct resources *res, const char *name);

#endif
