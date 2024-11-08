
#include <p2g/log.h>
#include <p2g/pad.h>
#include <p2g/core.h>

#include <stdlib.h>

int log_output_level = LOG_LEVEL_TRACE;

void p2g_fatal(const char *msg, ...) {
  logerr("FATAL: %s", msg);
  exit(-1);
}

int gs_init() {
  return 0;
}

int gs_flip() {
  return 0;
}

int gs_set_fields(int width, int height, int fmt, int zfmt, int fb1_addr,
    int fb2_addr, int zbuf_addr) {
  return 0;
}
int gs_set_output(int width, int height, int interlace, int mode, int ffmd,
    int filter_flicker) {
  return 0;
}
int gs_framebuffer_size(int width, int height, int psm) {
  return 0;
}

int gs_set_ztest(int mode) {
  return 0;
}

int draw_frame_start() {
  return 0;
}

int draw_frame_end() {
  return 0;
}

int draw2d_set_colour(unsigned char r, unsigned char g, unsigned char b,
    unsigned char a) {
  return 0;
}

int draw2d_bind_texture(int tex_vram_addr, int width, int height, int psm) {
  return 0;
}

int draw2d_sprite(float x, float y, float w, float h, float u1, float v1,
    float u2, float v2) {
  return 0;
}

int draw2d_rect(float x1, float y1, float w, float h) {
  return 0;
}

int draw_bind_buffer(void *buf, size_t buf_len) {
  return 0;
}

int draw2d_screen_dimensions(int w, int h) {
  return 0;
}

int draw_upload_texture(void *texture, size_t bytes, int width, int height,
    int format, int vram_addr) {
  return 0;
}

int draw2d_clear_colour(char r, char g, char b) {
  return 0;
}

int pad_init() {
  return 0;
}

void pad_frame_start() {
  return;
}

void pad_poll() {
  return;
}

static int held_ctr = 0;
int button_held(int i) {
  held_ctr = (held_ctr+1)%1000;
  if (i == DPAD_RIGHT && held_ctr < 500) {
    return 1;
  } 
  if (i == DPAD_LEFT && held_ctr >= 500) {
    return 1;
  }
  return 0;
}

int button_pressed(int i) {
  return 0;
}

int button_released(int i) {
  return 0;
}

void dma_channel_initialize(int, int, int) {}
void dma_channel_fast_waits(int) {}
void dma_wait_fast() {}
void draw_wait_finish() {}
void graph_wait_vsync() {}


