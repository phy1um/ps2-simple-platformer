#include <stdio.h>
#include <draw.h>
#include <graph.h>
#include <dma.h>
#include <gs_psm.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/gs.h>

#include <stdlib.h>

#define SCR_WIDTH 640
#define SCR_HEIGHT 448 

static uint32_t VRAM_HEAD = 0;
static uint32_t VRAM_SIZE = 4 * 1024 * 1024;
uint32_t vram_alloc(uint32_t size, uint32_t align) {
  trace("vram alloc: %d", size);
  while (VRAM_HEAD % align != 0) {
    VRAM_HEAD += 1;
  }
  uint32_t out = VRAM_HEAD;
  if (out > VRAM_SIZE) {
    p2g_fatal("VRAM overflow");
  }
  VRAM_HEAD += size;
  return out;
}

int main(int argc, char *argv[]) {
  logdbg("startup\n");

  gs_init();
  dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
  dma_channel_fast_waits(DMA_CHANNEL_GIF);

  gs_set_output(SCR_WIDTH, SCR_HEIGHT, GRAPH_MODE_INTERLACED, GRAPH_MODE_NTSC,
      GRAPH_MODE_FIELD, GRAPH_DISABLE);
  
  int fb1 = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int fb2 = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32), 2048);
  int zbuf = vram_alloc(gs_framebuffer_size(SCR_WIDTH, SCR_HEIGHT, GS_PSMZ_16), 2048);

  gs_set_fields(SCR_WIDTH, SCR_HEIGHT, GS_PSM_32, GS_PSMZ_16, fb1/4, fb2/4, zbuf/4);

  void *draw_buffer_ptr = calloc(1, 200*1024);
  draw_bind_buffer(draw_buffer_ptr, 200*1024);

  draw2d_screen_dimensions(SCR_WIDTH, SCR_HEIGHT);

  while(1) {
    // dma_wait_fast();
    draw_frame_start();
    draw2d_set_colour(255, 0, 0, 0x80);
    draw2d_rect(100., 80., 100., 100.);
    // draw stuff
    draw_frame_end();
    draw_wait_finish();
    graph_wait_vsync();
    gs_flip();
  }
}

