#include <stdio.h>
#include <p2g/log.h>

#include "../draw.h"
#include "../game/context.h"
#include "../vram.h"
#include "../levels/fmt.h"
#include "../io.h"
#include "../levels/levels.h"

int button_pressed(int b) {
  return 0;
}

int put_tile(struct ee_texture *tex, float tile_sq, float x, float y, float w, float h, int index) {
  return 0;
}
void draw_tile_map(struct tile_map *tm, float tile_sq, struct ee_texture *tex, struct game_camera *camera) {}

int draw_bind_texture(struct ee_texture *t) {
  return 0;
}
int draw_upload_ee_texture(struct ee_texture *t) {
  return 0;
}
int draw2d_set_colour(unsigned char r, unsigned char g, unsigned char b,
    unsigned char a) {return 0;}
int draw2d_rect(float x1, float y1, float w, float h) {return 0;}



int log_output_level = LOG_LEVEL_DEBUG;

int main(int argc, char *argv[]) {
  if (!io_init_wad(argv[1])) {
    logerr("open wad: %s", argv[1]);
    return 1;
  }

  struct vram_slice vram = {
    .start = 0,
    .end = VRAM_MAX,
  };
  vram_slice_reset_head(&vram);
  
  struct gamectx ctx = {0};

  ctx_init(&ctx, &vram);
  ctx_load_level(&ctx, fmt_load_level, "assets/entry_01.ps2lvl");

  return 0;
}
