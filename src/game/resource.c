#include <string.h>

#include "resource.h"
#include "hash.h"

int resource_insert_image(struct resources *res, const char *name, struct ee_texture *tex) {
  for (int i = 0; i < IMAGE_MAX; i++) {
    if (!res->images[i].active) {
      res->images[i].active = 1;
      res->images[i].name_hash = hash_fnv1a_32(name, strlen(name));
      res->images[i].tex = tex;
      return 0;
    }
  }
  return 1;
}

struct ee_texture *resource_get_image(struct resources *res, const char *name) {
  uint32_t hash = hash_fnv1a_32(name, strlen(name));
  for (int i = 0; i < IMAGE_MAX; i++) {
    if (res->images[i].active && res->images[i].name_hash == hash) {
      return res->images[i].tex;
    }
  }
  return 0;
}



