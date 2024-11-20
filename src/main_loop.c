#include <dma.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/gs.h>
#include <p2g/pad.h>

#include <stdlib.h>
#include <time.h>
#include <kernel.h>

#include "draw.h"
#include "tiles.h"
#include "io.h"
#include "vram.h"
#include "game/context.h"
#include "game/camera.h"
#include "game/player.h"
#include "menu/menu.h"

#include "levels/levels.h"
#include "levels/fmt.h"



void run_ctx_loop(struct gamectx *ctx) {
  int first_frame = 1;
  struct menu_state menu_state;
  int is_in_menu = 0;
  while(1) {
    pad_frame_start();
    pad_poll();
    trace("frame start - dma wait");
    // dma_wait_fast();
    draw_frame_start();
    if (first_frame) {
      if(draw_upload_ee_texture(ctx->game_font.texture)) {
        p2g_fatal("game font upload");
      }
      first_frame = 0;
    }
    struct entity *player = ctx_get_player(ctx);
    camera_focus(&ctx->camera, player->x, player->y);
    camera_debug(&ctx->camera);
    ctx_draw(ctx);
    if (is_in_menu) {
      menu_draw(&menu_state, &ctx->game_font, 10, 10);
    }
    logdbg("frame end");
    draw_frame_end();
    logdbg("sleep until vblank end");
    SleepThread();
    gs_flip();
    if (is_in_menu) {
      menu_drive_inputs(&menu_state, ctx);
    } else {
      ctx_update(ctx, 1.f/30.f);
      if (button_pressed(BUTTON_SELECT)) {
        // log_output_level = LOG_LEVEL_TRACE;
        menu_dbg_init(&menu_state, &is_in_menu);
      }
    }
  }
}
