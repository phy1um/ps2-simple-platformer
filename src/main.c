#include <stdio.h>
#include <draw.h>
#include <graph.h>
#include <dma.h>
#include <gs_psm.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/gs.h>

#include <stdlib.h>

#include "draw.h"

int main(int argc, char *argv[]) {
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

  // TODO: implement :)
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
    for (int i = 0; i < 10; i++) {
      put_tile(i*24., 100., 24., 24., 12+i);
    }
    trace("frame end");
    draw_frame_end();
    trace("draw wait");
    draw_wait_finish();
    trace("vsync wait");
    graph_wait_vsync();
    trace("gs flip buffers");
    gs_flip();
  }
}

