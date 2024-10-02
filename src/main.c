#include <stdio.h>
#include <draw.h>
#include <graph.h>
#include <dma.h>
#include <gs_psm.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/gs.h>

#include <stdlib.h>
#include <time.h>

#include "draw.h"
#include "tiles.h"
#include "game/context.h"
#include "game/player.h"

int main(int argc, char *argv[]) {
  srand(time(NULL));
  log_output_level = LOG_LEVEL_DEBUG;
  logdbg("startup\n");

  trace("gs init");
  gs_init();
  dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
  dma_channel_fast_waits(DMA_CHANNEL_GIF);

  gs_set_output(SCR_WIDTH, SCR_HEIGHT, GRAPH_MODE_INTERLACED, GRAPH_MODE_NTSC,
      GRAPH_MODE_FIELD, GRAPH_DISABLE);
  
  trace("allocating framebuffers");
  int fb1 = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int fb2 = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int zbuf = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSMZ_16), 2048);

  trace("using framebuffers");
  gs_set_fields(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32, GS_PSMZ_16, fb1/4, fb2/4, zbuf/4);

  void *draw_buffer_ptr = calloc(1, 200*1024);
  draw_bind_buffer(draw_buffer_ptr, 200*1024);

  trace("setting screen dimensions: %dx%d", SCR_WIDTH, SCR_HEIGHT);
  draw2d_screen_dimensions(SCR_WIDTH, SCR_HEIGHT);

  struct gamectx ctx = {0};
  size_t player_index = 0;
  if (ctx_next_entity(&ctx, &player_index)) {
    logerr("man really went wrong here");
    p2g_fatal("dead on arrival");
    return 1;
  }
  struct entity *player = &(ctx.entities[player_index]);
  player_new(player, 30., 30.);

  struct tile_map tm;
  tile_map_init(&tm, 40, 28, 0, 0);
  for(int yy = 0; yy < tm.height; yy++) {
    for(int xx = 0; xx < tm.width; xx++) {
      int r = rand() % 40;
      logdbg("put tile %d @ %d,%d", r, xx, yy);
      if (r < 8) {
        set_tile(&tm, xx, yy, r);
      }
    }
  }

  load_textures();
  draw2d_clear_colour(33, 38, 63);
  while(1) {
    trace("frame start - dma wait");
    dma_wait_fast();
    draw_frame_start();
    trace("upload textures");
    upload_textures();
    trace("bind tileset");
    bind_tileset();
    trace("put tile");
    draw2d_set_colour(0x80, 0x80, 0x80, 0x80);
    draw_tile_map(&tm);
    entity_draw_list(ctx.entities, ENTITY_MAX, &ctx);
    trace("frame end");
    draw_frame_end();
    trace("draw wait");
    draw_wait_finish();
    trace("vsync wait");
    graph_wait_vsync();
    trace("gs flip buffers");
    gs_flip();
    entity_update_list(ctx.entities, ENTITY_MAX, &ctx, 1./30.);
  }
}

