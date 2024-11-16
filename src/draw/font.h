
#ifndef SRC_DRAW_FONT_H
#define SRC_DRAW_FONT_H

#include <stddef.h>

struct ee_font {
  struct ee_texture *texture; 
  // pixels w/h per character
  int char_dims[2];
  // how much to post-scale the font
  float aspect_correct;
  int chars_per_line;
  float dtex[2];
};

int font_init(struct ee_font *fnt, struct ee_texture *tex, int dimx, int dimy, int inner_width, int inner_height, float aspect_correct);
int font_putch(struct ee_font *fnt, char c, float xp, float yp, float scale);
int font_putstr(struct ee_font *fnt, const char *str, size_t len, float xp, float yp, float scale);

#endif
