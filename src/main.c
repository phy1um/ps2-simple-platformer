#include <stdio.h>
#include <draw.h>
#include <graph.h>
#include <dma.h>
#include <gs_psm.h>
#include <gs_privileged.h>

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
#include "task.h"

#include "levels/levels.h"
#include "levels/fmt.h"

#ifndef VBLANK_TIMEOUT_MAX
#define VBLANK_TIMEOUT_MAX 999999
#endif

void run_ctx_loop(struct gamectx *ctx);

unsigned char ctx_stack[0x800] __attribute__((aligned(64)));
static s32 main_cleanup_threadid = 0;
static s32 main_event_loop_threadid = 0;

static int vblank_without_draw = 0;
static int overseer_vblank_handler(int u) {
    if (!(*GS_REG_CSR & 2)) {
      if (vblank_without_draw > VBLANK_TIMEOUT_MAX) {
        iWakeupThread(main_cleanup_threadid);
      }
      vblank_without_draw += 1;
      iRotateThreadReadyQueue(15);
    } else {
      vblank_without_draw = 0;
      *GS_REG_CSR |= 2;
      iWakeupThread(main_event_loop_threadid);
  }
  ExitHandler();
  return 0;
}

int register_wake_on_vblank() {
  info("register vblank handler");
  int rv = AddIntcHandler(2, overseer_vblank_handler, 0);
  if (rv == -1) {
    return 1;
  }
  EnableIntc(2);
  return 0;
}

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

  if (task_system_init()) {
    p2g_fatal("task system setup");
    return 1;
  }

  struct gamectx ctx = {0};

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

  if (ctx_init(&ctx, &vram)) {
    p2g_fatal("init context");
    return 1;
  }
  ctx_load_level(&ctx, fmt_load_level, "assets/entry_01.ps2lvl");
  ctx_swap_active_level(&ctx);

  gs_set_ztest(2);

  if (register_wake_on_vblank()) {
    p2g_fatal("failed to setup VBLANK intc handler"); 
    return 1;
  }

  draw2d_clear_colour(33, 38, 63);

  main_cleanup_threadid = GetThreadId();

  ee_thread_t ctx_loop = {
    .func = run_ctx_loop,
    .stack = ctx_stack,
    .stack_size = sizeof(ctx_stack),
    .gp_reg = &_gp,
    .initial_priority = 5,
  };
  main_event_loop_threadid = CreateThread(&ctx_loop);
  StartThread(main_event_loop_threadid, &ctx); 
  SleepThread();
  p2g_fatal("main loop dead");
}

