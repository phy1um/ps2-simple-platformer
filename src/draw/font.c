#include <p2g/ps2draw.h>
#include <p2g/log.h>

#include "font.h"
#include "../draw.h"

int font_init(struct ee_font *fnt, struct ee_texture *tex, int dimx, int dimy, int inner_width, int inner_height, float aspect_correct) {
  if (!fnt) {
    logerr("init NULL font");
    return 1;
  }
  if (!tex) {
    logerr("init font with NULL texture");
    return 1;
  }
  if (dimx == 0 || dimy == 0) {
    logerr("font character dimensions: %dx%d", dimx, dimy);
    return 1;
  }
  fnt->texture = tex;
  fnt->char_dims[0] = dimx;
  fnt->char_dims[1] = dimy;
  fnt->aspect_correct = aspect_correct;
  fnt->chars_per_line = inner_width / dimx;
  fnt->dtex[0] = ((float)dimx)/((float)tex->width);
  fnt->dtex[1] = ((float)dimy)/((float)tex->height);
  logdbg("loaded font: %dx%d, (texture=%dx%d) chars per line = %d, du/dv = %f/%f", dimx, dimy, tex->width, tex->height,
      fnt->chars_per_line, fnt->dtex[0], fnt->dtex[1]);
  return 0;
}

int font_putch(struct ee_font *fnt, char c, float xp, float yp, float scale) {
  if (!fnt) {
    logerr("putch NULL font");
    return 1;
  }
  char index = c;
  int index_x = index % fnt->chars_per_line;
  int index_y = index / fnt->chars_per_line;
  float du = fnt->dtex[0];
  float dv = fnt->dtex[1];
  float u0 = index_x*du;
  float u1 = (index_x+1)*du;
  float v0 = index_y*dv;
  float v1 = (index_y+1)*dv;
  trace("putch %c @ %f scale [index %d = %dx%d] [uvs = %f,%f,%f,%f]", c, scale, index, index_x, index_y,
      u0, v0, u1, v1);
  return (!put_sprite(fnt->texture, xp, yp, fnt->char_dims[0]*scale, fnt->char_dims[1]*scale, 
      u0, v0, u1, v1));
}

int font_putstr(struct ee_font *fnt, const char *str, size_t len, float xp, float yp, float scale) {
  float xp_iter = xp;
  float yp_iter = yp;
  draw_bind_texture(fnt->texture);
  trace("putstr: %s (%zu) @ %f, %f (x%f)", str, len, xp, yp, scale);
  for (int i = 0; i < len; i++) {
    char c = str[i];
    if (c == '\n') {
      yp_iter += fnt->char_dims[1];
      continue;
    }
    if (font_putch(fnt, c, xp_iter, yp_iter, scale)) {
      logerr("font putch: %c @ %f,%f", str[i], xp_iter, yp_iter);
      return 1;
    }
    xp_iter += fnt->char_dims[0]*scale;
  }
  trace("putstr done");
  return 0;
}

