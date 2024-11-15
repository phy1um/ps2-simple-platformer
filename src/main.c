#include <stdio.h>
#include <draw.h>
#include <graph.h>
#include <dma.h>
#include <gs_psm.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/gs.h>
#include <p2g/pad.h>

#include <stdlib.h>
#include <time.h>

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

int main(int argc, char *argv[]) {
  srand(time(NULL));
  log_output_level = LOG_LEVEL_DEBUG;
  logdbg("startup");

  pad_init();

  trace("gs init");
  gs_init();
  dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
  dma_channel_fast_waits(DMA_CHANNEL_GIF);

  gs_set_output(SCR_WIDTH, SCR_HEIGHT, GRAPH_MODE_INTERLACED, GRAPH_MODE_NTSC,
      GRAPH_MODE_FIELD, GRAPH_DISABLE);

  struct vram_slice vram = {
    .start = 0,
    .end = VRAM_MAX,
  };
  vram_slice_reset_head(&vram);
  
  trace("allocating framebuffers");
  int fb1 = vram_alloc(&vram, gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int fb2 = vram_alloc(&vram, gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int zbuf = vram_alloc(&vram, gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSMZ_16S), 2048);
  logdbg("vram head after framebuffer allocations: %X (%X words)", vram.head, vram.head/4);

  trace("using framebuffers");
  gs_set_fields(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32, GS_PSMZ_16S, fb1/4, fb2/4, zbuf/4);

  void *draw_buffer_ptr = calloc(1, 200*1024);
  draw_bind_buffer(draw_buffer_ptr, 200*1024);

  trace("setting screen dimensions: %dx%d", SCR_WIDTH, SCR_HEIGHT);
  draw2d_screen_dimensions(SCR_WIDTH, SCR_HEIGHT);

  if (!io_init_wad("./assets.wad")) {
    logerr("load assets");
    p2g_fatal("no assets to work with");
    return 1;
  }
  struct gamectx ctx = {0};
  struct menu_state menu_state;
  int is_in_menu = 0;

  size_t player_index = 0;
  if (ctx_next_entity(&ctx, &player_index)) {
    logerr("man really went wrong here");
    p2g_fatal("dead on arrival");
    return 1;
  }
  struct entity *player = &(ctx.entities[player_index]);

  player_new(player, 200., 200.);

  float cam_bounds[] = {640., 448.};
  float cam_fbox[] = {50., 50.};
  camera_init(&ctx.camera, cam_bounds, cam_fbox);

  ctx_init(&ctx, &vram);
  ctx_load_level(&ctx, fmt_load_level, "assets/entry_01.ps2lvl");
  ctx_swap_active_level(&ctx);

  gs_set_ztest(2);

  // load_textures(&vram);
  draw2d_clear_colour(33, 38, 63);
  while(1) {
    pad_frame_start();
    pad_poll();
    trace("frame start - dma wait");
    dma_wait_fast();
    draw_frame_start();
    camera_focus(&ctx.camera, player->x, player->y);
    camera_debug(&ctx.camera);
    ctx_draw(&ctx);
    if (is_in_menu) {
      menu_draw(&menu_state, 10, 10);
    }
    trace("frame end");
    draw_frame_end();
    trace("draw wait");
    draw_wait_finish();
    trace("vsync wait");
    graph_wait_vsync();
    trace("gs flip buffers");
    gs_flip();
    if (is_in_menu) {
      menu_drive_inputs(&menu_state, &ctx);
    } else {
      ctx_update(&ctx, 1.f/30.f);
      if (button_pressed(BUTTON_SELECT)) {
        menu_dbg_init(&menu_state, &is_in_menu);
      }
    }
  }
}

