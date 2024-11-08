
#ifndef SRC_LEVELS_FMT_H
#define SRC_LEVELS_FMT_H

#define NAME_LEN 24
#define FILE_VERSION 1
#define FILE_ID = "LVL2"

#include <stdint.h>
#include "../game/context.h"

enum tilemap_kind {
  MAP_COLLISION,
  MAP_DECO,
};

struct __attribute__((packed)) level_header {
  char id[4];
  uint16_t version;
  uint16_t tilemap_def_count;
  uint16_t asset_def_count;
  uint16_t entity_def_count;
  int32_t world_offset[2];
  char short_name[NAME_LEN];
};

struct __attribute__((packed)) level_tilemap_def {
  int32_t local_offset[2];
  uint32_t size[2];
  uint16_t tilemap_kind;
  uint32_t asset_ref;
  uint32_t map_file_offset;
};

struct __attribute__((packed)) asset_def {
  uint16_t asset_kind;
  uint32_t file_offset_name;
  uint32_t name_len;
};

int level_load(const char *fname, struct gamectx *ctx, struct levelctx *lvl);

#endif
