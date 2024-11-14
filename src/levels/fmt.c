#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <string.h>

#include "fmt.h"

#include "../tga.h"
#include "../tiles.h"
#include "levels.h"
#include "../draw.h"
#include "../io.h"
#include "../game/context.h"
#include "../game/trigger.h"

struct loaded_deco {
  float position[2];
  uint32_t bounds[2];
  size_t texture_index;
  uint16_t texture_region[4];
};

struct loaded_tile_map {
  enum tilemap_kind kind;
  uint32_t texture_index;
  struct tile_map tiles;
};

struct level_loaded {
  char short_name[NAME_LEN]; 
  uint16_t tilemap_count;
  uint16_t texture_count;
  uint16_t area_count;
  uint16_t deco_count;
  int deco_map_tex_ref;
  struct loaded_tile_map *maps;
  struct ee_texture *textures;
  struct trigger_area *areas;
  struct loaded_deco *decos;
};

static int map_biggest_dims(struct level_tilemap_def *ld, size_t count, int *w, int *h) {
  *w = 0;
  *h = 0;
  for (int i = 0; i < count; i++) {
    int gw = ld[i].size[0]*GRID_SIZE;
    if (gw > *w) {
      logdbg("biggest dim w: %d", gw);
      *w = gw;
    }
    int gh = ld[i].size[1]*GRID_SIZE;
    if (gh > *h) {
      logdbg("biggest dim h: %d", gh);
      *h = gh;
    }
  }
  return 0;
}


static int loaded_reload(struct gamectx *ctx, struct levelctx *lvl);
static int loaded_draw(struct gamectx *ctx, struct levelctx *lvl);
static int loaded_update(struct gamectx *ctx, struct levelctx *lvl, float dt);
static int loaded_test_point(struct levelctx *lvl, float x, float y);

int load_tga(const char *fname, struct levelctx *lvl, struct ee_texture *tgt);

int load_level_areas(
    const char *fname,
    struct level_header *header,
    struct level_area_def *area_defs,
    struct level_loaded *loaded,
    struct levelctx *lvl
) {
  loaded->areas = alloc_from(&lvl->allocator, header->area_def_count, sizeof(struct trigger_area));
  if (!loaded->areas) {
    logerr("alloc loaded decos: %s", fname);
    return 1;
  }
  for (int i = 0; i < header->area_def_count; i++) {
    struct level_area_def *a = &area_defs[i]; 
    logdbg("loading area (%d): kind=%lu, arg offset=%lu, arg len=%lu", i,
        a->kind, a->arg_file_offset, a->arg_len);
    char *arg = alloc_from(&lvl->allocator, 1, a->arg_len+1);
    if (!arg) {
      logerr("failed to alloc arg buffer");
      return 1;
    }
    int rc = io_read_file_part(fname, arg, a->arg_len, a->arg_file_offset, a->arg_file_offset+ a->arg_len);
    arg[a->arg_len] = 0;
    if (!rc) {
      logerr("load level %s: area %i read arg @ %lu (len=%lu)", fname, i,
          a->arg_file_offset, a->arg_len);
      return 1;
    }
    struct trigger_area *la = &loaded->areas[i];
    la->position[0] = a->local_offset[0] + header->world_offset[0];
    la->position[1] = a->local_offset[1] + header->world_offset[1];
    la->size[0] = a->bounds[0];
    la->size[1] = a->bounds[1];
    la->kind = a->kind;
    la->arg = arg;
    la->arg_len = a->arg_len;
  }
  return 0;
}

int load_assets(
    const char *fname,
    struct level_header *header,
    struct asset_def *asset_defs,
    struct level_loaded *loaded,
    struct levelctx *lvl
) {
  loaded->textures = alloc_from(&lvl->allocator, header->asset_def_count, sizeof(struct ee_texture));
  if (!loaded->textures) {
    logerr("alloc loaded textures: %s", fname);
    return 1;
  }
  for (int i = 0; i < header->asset_def_count; i++) {
      struct asset_def *a = &asset_defs[i];
      logdbg("loading asset (%d): kind=%d, name offset=%lu, name len=%lu", i,
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

int load_level_decos(
    const char *fname,
    struct level_header *header,
    struct level_loaded *loaded, 
    struct level_deco_def *deco_defs,
    struct levelctx *lvl
) {
  loaded->decos = alloc_from(&lvl->allocator, header->deco_def_count, sizeof(struct loaded_deco));
  for (int i = 0; i < header->deco_def_count; i++) {
    struct loaded_deco *tgt = &loaded->decos[i];
    struct level_deco_def *def = &deco_defs[i];
    tgt->position[0] = def->local_offset[0] + header->world_offset[0];
    tgt->position[1] = def->local_offset[1] + header->world_offset[1];
    tgt->bounds[0] = def->bounds[0];
    tgt->bounds[1] = def->bounds[1];
    tgt->texture_index = def->asset_ref;
    memcpy(tgt->texture_region, def->uvs, sizeof(uint16_t)*4);
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
  loaded->maps = alloc_from(&lvl->allocator, header->tilemap_def_count, sizeof(struct loaded_tile_map));
  if (!loaded->maps) {
    logerr("alloc loaded tilemaps: %s", fname);
    return 1;
  }
  for (int i = 0; i < header->tilemap_def_count; i++) {
    struct level_tilemap_def *t = &tilemap_defs[i];
    struct loaded_tile_map *tgt = &loaded->maps[i];
    enum tilemap_kind kind = (enum tilemap_kind) t->tilemap_kind;
    tgt->kind = kind;
    if (t->asset_ref != UINT32_MAX) {
      if (t->asset_ref > header->asset_def_count) {
        logerr("load level %s: map %d references asset %ld (/%d)", fname, 
            i, t->asset_ref, header->asset_def_count);
        return 1;
      }
      tgt->texture_index = t->asset_ref;
    }
    tile_map_init(&tgt->tiles, t->size[0], t->size[1], GRID_SIZE, 
        header->world_offset[0]+t->local_offset[0], 
        header->world_offset[1]+t->local_offset[1], 
        &lvl->allocator);
    uint32_t map_size = t->size[0]*t->size[1];
    logdbg("reading tilemap (%d) @ %lu for %lu into -> %p", kind, t->map_file_offset, map_size, tgt->tiles.tiles);
    io_read_file_part(fname, tgt->tiles.tiles, map_size, t->map_file_offset, 
        t->map_file_offset + map_size);
  }
  return 0;
}

int load_tga(const char *fname, struct levelctx *lvl, struct ee_texture *tgt) {
  struct tga_data tga;
  int rc = tga_from_file(fname, &tga, &lvl->allocator);
  if (rc) {
    logerr("failed to load \"%s\"", fname);
    return 1;
  }
  tgt->pixels = tga.pixels;
  tgt->size = tga.pixels_size;
  tgt->width = tga.header.width;
  tgt->height = tga.header.height;
  tgt->vram_addr = vram_alloc(&lvl->vram, tga.pixels_size, 2048)/4;
  return 0;
}

int fmt_load_level(struct gamectx *ctx, struct levelctx *lvl, const char *fname) {
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

  // =====================
  // read defs 
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
  read_head += header.asset_def_count* sizeof(struct asset_def);

  struct level_area_def *area_defs = alloc_from(&lvl->allocator, 
        header.area_def_count, sizeof(struct level_area_def));
  if (header.area_def_count > 0) {
      if (!area_defs) {
      logerr("alloc area defs: %s", fname);
    }
    logdbg("read area defs @ %X [+ %X]", read_head,
        header.area_def_count * sizeof(struct level_area_def));
    rc = io_read_file_part(fname, area_defs, 
        header.area_def_count * sizeof(struct level_area_def),
        read_head, 
        read_head + header.area_def_count * sizeof(struct level_area_def));
    if (!rc) {
      logerr("read area defs: %s", fname);
      return 1;
    }
    logdbg("area defs size = %zu", header.area_def_count * sizeof(struct level_area_def));
  }

  read_head += header.area_def_count* sizeof(struct level_area_def);

  struct level_deco_def *deco_defs = alloc_from(&lvl->allocator,
        header.deco_def_count, sizeof(struct level_deco_def));
  if (header.deco_def_count > 0) {
      if (!deco_defs) {
      logerr("alloc deco defs: %s", fname);
    }
    logdbg("read deco defs @ %X [+ %X]", read_head,
        header.deco_def_count * sizeof(struct level_deco_def));
    rc = io_read_file_part(fname, deco_defs, 
        header.deco_def_count * sizeof(struct level_deco_def),
        read_head, 
        read_head + header.deco_def_count * sizeof(struct level_deco_def));
    if (!rc) {
      logerr("read deco defs: %s", fname);
      return 1;
    }
    logdbg("deco defs size = %zu", header.deco_def_count * sizeof(struct level_deco_def));
  }

  // =====================
  // read + init data
  if (load_assets(fname, &header, asset_defs, loaded, lvl)) {
    logerr("load level assets: %s", fname);
    return 1;
  }
  if (load_level_maps(fname, &header, loaded, tilemap_defs, lvl)) {
    logerr("load level maps: %s", fname);
    return 1;
  }
  if (load_level_areas(fname, &header, area_defs, loaded, lvl)) {
    logerr("load level trigger areas: %s", fname);
    return 1;
  }
  if (load_level_decos(fname, &header, loaded, deco_defs, lvl)) {
    logerr("load level decos: %s", fname);
    return 1;
  }

  loaded->texture_count = header.asset_def_count;
  loaded->tilemap_count = header.tilemap_def_count;
  loaded->area_count = header.area_def_count;
  loaded->deco_count = header.deco_def_count;
  memcpy(loaded->short_name, header.short_name, NAME_LEN);
  lvl->loaded_name = alloc_from(&lvl->allocator, 1, strlen(fname));
  if (!lvl->loaded_name) {
    logerr("load level: loaded name alloc failed: %s", fname);
    return 1;
  }
  strcpy(lvl->loaded_name, fname);
  lvl->leveldata = loaded;
  lvl->update = loaded_update;
  lvl->test_point = loaded_test_point;
  lvl->reload = loaded_reload;
  lvl->cleanup = 0;
  lvl->draw = loaded_draw;
  lvl->active = 1;
  int max_width, max_height;
  map_biggest_dims(tilemap_defs, header.tilemap_def_count, &max_width, &max_height);
  int size_x = header.world_offset[0] + max_width;
  int size_y = header.world_offset[1] + max_height;
  lvl->bounds[0] = header.world_offset[0];
  lvl->bounds[1] = header.world_offset[1];
  lvl->bounds[2] = size_x;
  lvl->bounds[3] = size_y;
  logdbg("loaded level bounds: [%f %f %f %f]",
      lvl->bounds[0],
      lvl->bounds[1],
      lvl->bounds[2],
      lvl->bounds[3]);
  logdbg("finished loading level: %s (# maps = %d)", header.short_name, header.tilemap_def_count);
  return 0;
}

static int loaded_update(struct gamectx *ctx, struct levelctx *lvl, float dt) {
  struct level_loaded *loaded = (struct level_loaded *)lvl->leveldata;
  struct entity *player = ctx_get_player(ctx); 
  for (int i = 0; i < loaded->area_count; i++) {
    struct trigger_area *area = &loaded->areas[i];
    if (trigger_collides_point(area, player->x, player->y)) {
      trigger_event(area, ctx, lvl);
    }
  }
  return 0;
}

static int loaded_test_point(struct levelctx *lvl, float x, float y) {
  struct level_loaded *loaded = (struct level_loaded *)lvl->leveldata;
  if (!loaded) {
    logerr("leveldata null");
  }
  for (int i = 0; i < loaded->tilemap_count; i++) {
    struct loaded_tile_map *tm = &loaded->maps[i]; 
    if (tm->kind != MAP_COLLISION) {
      continue; 
    }
    char hit = get_tile_world(&tm->tiles, x, y);
    if (hit != 0 && hit != TILE_INVALID) {
      return hit;
    }
  }
  return 0;
}

static int loaded_draw(struct gamectx *ctx, struct levelctx *lvl) {
  struct level_loaded *loaded = (struct level_loaded *)lvl->leveldata;
  if (!loaded) {
    logerr("leveldata null");
  }
  for (int i = 0; i < loaded->texture_count; i++) {
    struct ee_texture *tex = &loaded->textures[i];
    draw_upload_ee_texture(tex);
  }
  for (int i = 0; i < loaded->tilemap_count; i++) {
    struct loaded_tile_map *tm = &loaded->maps[i]; 
    if (tm->kind != MAP_DECO) {
      continue; 
    }
    if (tm->texture_index == UINT32_MAX) {
      logerr("draw tilemap %d: invalid texture", i);
      continue; 
    }
    if (tm->texture_index >= loaded->texture_count) {
      logerr("draw tilemap %d: texture oob (%lu/%lu)", i, tm->texture_index, loaded->texture_count);
      continue;
    }
    struct ee_texture *tex = &loaded->textures[tm->texture_index];
    draw_bind_texture(tex);
    draw_tile_map(&tm->tiles, GRID_SIZE, tex, &ctx->camera);
  }
  for (int i = 0; i < loaded->deco_count; i++) {
    struct loaded_deco *d = &loaded->decos[i];
    if (d->texture_index == UINT32_MAX) {
      logerr("draw deco %d: invalid texture", i);
      continue; 
    }
    if (d->texture_index >= loaded->texture_count) {
      logerr("draw deco %d: texture oob (%d/%d)", i, d->texture_index, loaded->texture_count);
      continue;
    }
    struct ee_texture *tex = &loaded->textures[d->texture_index];
    draw_bind_texture(tex);
    
    float u0 = ((float)d->texture_region[0])/((float)tex->width);
    float v0 = ((float)d->texture_region[1])/((float)tex->height);
    float u1 = ((float)d->texture_region[2])/((float)tex->width);
    float v1 = ((float)d->texture_region[3])/((float)tex->height);

    float ax = d->position[0] - ctx->camera.position[0];
    float ay = d->position[1] - ctx->camera.position[1];

    if (!put_sprite(tex, ax, ay, d->bounds[0], d->bounds[1], u0, v0, u1, v1)) {
      logerr("draw deco %d: put sprite", i);
    }
  }
#ifdef DEBUG_DRAW_AREAS
  // TODO: runtime toggle to draw trigger areas
  for (int i = 0; i < loaded->area_count; i++) {
    struct trigger_area area = loaded->areas[i];
    draw2d_set_colour(0xf0, 0x0e, 0x20, 0x20);
    float ax = area.position[0] - ctx->camera.position[0];
    float ay = area.position[1] - ctx->camera.position[1];
    draw2d_rect(ax, ay, area.size[0], area.size[1]);
  }
#endif
  return 0;
}

static int loaded_reload(struct gamectx *ctx, struct levelctx *lvl) {
  vram_slice_reset_head(&lvl->vram);
  lvl->heap_head = 0;
  char fname[120];
  strncpy(fname, lvl->loaded_name, 120);
  return fmt_load_level(ctx, lvl, fname);
}


