#include <p2g/log.h>
#include <string.h>

#include "fmt.h"

#include "../tga.h"
#include "../tiles.h"
#include "levels.h"
#include "../draw.h"
#include "../io.h"
#include "../game/context.h"

struct level_loaded {
  char short_name[NAME_LEN]; 
  uint16_t tilemap_count;
  uint16_t texture_count;
  int deco_map_tex_ref;
  struct tile_map *maps;
  struct ee_texture *textures;
};

static int loaded_draw(struct gamectx *ctx, struct levelctx *lvl);
int load_assets(
    const char *fname,
    struct level_header *header,
    struct asset_def *asset_defs,
    struct level_loaded *loaded,
    struct levelctx *lvl);
int load_level_maps(
    const char *fname, 
    struct level_header *header,
    struct level_loaded *loaded, 
    struct level_tilemap_def *tilemap_defs, 
    struct levelctx *lvl);
int load_tga(const char *fname, struct levelctx *lvl, struct ee_texture *tgt);

int level_load(const char *fname, struct gamectx *ctx, struct levelctx *lvl) {
  // read header and defs
  struct level_loaded *loaded = alloc_from(&lvl->allocator, 1, sizeof(struct level_loaded));
  if (!loaded) {
    logerr("alloc loaded level struct");
    return 1;
  }
  struct level_header header = {0};
  size_t rc = io_read_file_part(fname, &header, sizeof(struct level_header), 0, sizeof(struct level_header));
  if (!rc) {
    logerr("load level %s", fname);
    return 1;
  }
  struct level_tilemap_def *tilemap_defs = alloc_from(&lvl->allocator, 
      header.tilemap_def_count, sizeof(struct level_tilemap_def));
  if (!tilemap_defs) {
    logerr("alloc tilemap definitions: %s", fname);
    return 1;
  }
  size_t read_head = sizeof(struct level_header);
  logdbg("read tilemap defs @ %X [+ %X]", read_head,
      header.tilemap_def_count * sizeof(struct level_tilemap_def));
  rc = io_read_file_part(fname, tilemap_defs, 
      header.tilemap_def_count * sizeof(struct level_tilemap_def),
      read_head, 
      read_head + header.tilemap_def_count * sizeof(struct level_tilemap_def));
  if (!rc) {
    logerr("read tilemap defs: %s", fname);
    return 1;
  }
  read_head += header.tilemap_def_count * sizeof(struct level_tilemap_def);

  struct asset_def *asset_defs = alloc_from(&lvl->allocator,
      header.asset_def_count, sizeof(struct asset_def));
  if (!asset_defs) {
    logerr("alloc asset definitions: %s", fname);
    return 1;
  }
  logdbg("read asset defs @ %X [+ %X]", read_head,
      header.asset_def_count * sizeof(struct asset_def));
  rc = io_read_file_part(fname, asset_defs, 
      header.asset_def_count * sizeof(struct asset_def),
      read_head, 
      read_head + header.asset_def_count * sizeof(struct asset_def));
  if (!rc) {
    logerr("read assetmap defs: %s", fname);
    return 1;
  }
  loaded->textures = alloc_from(&lvl->allocator, header.asset_def_count, sizeof(struct ee_texture));
  if (!loaded->textures) {
    logerr("alloc loaded textures: %s", fname);
    return 1;
  }
  loaded->maps = alloc_from(&lvl->allocator, header.tilemap_def_count, sizeof(struct tile_map));
  if (!loaded->maps) {
    logerr("alloc loaded tilemaps: %s", fname);
    return 1;
  }
  if (load_assets(fname, &header, asset_defs, loaded, lvl)) {
    logerr("load level assets: %s", fname);
    return 1;
  }
  if (load_level_maps(fname, &header, loaded, tilemap_defs, lvl)) {
    logerr("load level maps: %s", fname);
    return 1;
  }
  loaded->texture_count = header.asset_def_count;
  loaded->tilemap_count = header.tilemap_def_count;
  memcpy(loaded->short_name, header.short_name, NAME_LEN);
  lvl->leveldata = loaded;
  lvl->update = 0;
  lvl->cleanup = 0;
  lvl->draw = loaded_draw;
  lvl->active = 1;
  logdbg("finished loading level: %s", header.short_name);
  return 0;
}

static int loaded_draw(struct gamectx *ctx, struct levelctx *lvl) {
  struct level_loaded *loaded = (struct level_loaded *)lvl->leveldata;
  if (!loaded) {
    logerr("leveldata null");
  }
  struct ee_texture *tex = &loaded->textures[loaded->deco_map_tex_ref];
  draw_upload_ee_texture(tex);
  draw_bind_texture(tex);
  draw_tile_map(&lvl->decoration, 16, tex, &ctx->camera);
  return 0;
}

int load_assets(
    const char *fname,
    struct level_header *header,
    struct asset_def *asset_defs,
    struct level_loaded *loaded,
    struct levelctx *lvl
) {
  for (int i = 0; i < header->asset_def_count; i++) {
      struct asset_def *a = &asset_defs[i];
      logdbg("loading asset (%d): kind=%d, name offset=%d, name len=%d", i,
          a->asset_kind, a->file_offset_name, a->name_len);
      char *name = alloc_from(&lvl->allocator, 1, a->name_len + 1); 
      if (!name) {
        logerr("failed to allocate name buffer");
        return 1;
      }
      int rc = io_read_file_part(fname, name, a->name_len, a->file_offset_name, a->file_offset_name + a->name_len);
      name[a->name_len] = 0;
      if (!rc) {
        logerr("load level %s: asset %i read name @ %ld (len=%ld)", fname, i,
            a->file_offset_name, a->name_len);
        return 1;
      }
      if (load_tga(name, lvl, &loaded->textures[i])) {
        logerr("load tga: %s @ slot %d", name, i);
        return 1;
      }
  }
  return 0;
}

int load_level_maps(
    const char *fname, 
    struct level_header *header,
    struct level_loaded *loaded, 
    struct level_tilemap_def *tilemap_defs, 
    struct levelctx *lvl
) {
  int has_deco = 0;
  int has_col = 0;
  for (int i = 0; i < header->tilemap_def_count; i++) {
    struct level_tilemap_def *t = &tilemap_defs[i];
    struct tile_map *tgt = 0;
    enum tilemap_kind kind = (enum tilemap_kind) t->tilemap_kind;
    if (kind == MAP_DECO && !has_deco) {
      tgt = &lvl->decoration; 
      has_deco = 1;
    }
    if (kind == MAP_COLLISION && !has_col) {
      tgt = &lvl->collision; 
      has_col = 1;
    }
    if (!tgt) {
      continue;
    }
    if (t->asset_ref != UINT32_MAX) {
      if (t->asset_ref > header->asset_def_count) {
        logerr("load level %s: map %d references asset %ld (/%d)", fname, 
            i, t->asset_ref, header->asset_def_count);
        return 1;
      }
      loaded->deco_map_tex_ref = t->asset_ref;
    }
    tile_map_init(tgt, t->size[0], t->size[1], GRID_SIZE, 
        header->world_offset[0]+t->local_offset[0], 
        header->world_offset[1]+t->local_offset[1], 
        &lvl->allocator);
    uint32_t map_size = t->size[0]*t->size[1];
    logdbg("reading tilemap (%d) @ %d for %d", kind, t->map_file_offset, map_size);
    io_read_file_part(fname, tgt->tiles, map_size, t->map_file_offset, 
        t->map_file_offset + map_size);
  }
  if (!has_deco || !has_col) {
    logerr("must have at least 1 collision and 1 decoration map: %s", fname);
    return 1;
  }
  return 0;
}

int load_tga(const char *fname, struct levelctx *lvl, struct ee_texture *tgt) {
  struct tga_data tga;
  int rc = tga_from_file(fname, &tga, &lvl->allocator);
  if (rc) {
    logerr("failed to load tiles_test0.tga");
    return 1;
  }
  tgt->pixels = tga.pixels;
  tgt->size = tga.pixels_size;
  tgt->width = tga.header.width;
  tgt->height = tga.header.height;
  tgt->vram_addr = vram_alloc(&lvl->vram, tga.pixels_size, 2048)/4;
  return 0;

}
