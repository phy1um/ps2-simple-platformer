#include <stdio.h>
#include <p2g/log.h>

#include "../draw.h"
#include "../game/context.h"
#include "../vram.h"
#include "../levels/fmt.h"
#include "../io.h"
#include "../levels/levels.h"

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
  ctx_load_level(&ctx, fmt_load_level, "assets/entry_02.ps2lvl");

  return 0;
}
