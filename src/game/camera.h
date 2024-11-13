#ifndef SRC_GAME_CAMERA_H
#define SRC_GAME_CAMERA_H

struct game_camera {
  float position[2];
  float bounds[2]; 
  float focus_box[2];
  float scale;
  char blend_colour[4];
};

int camera_init(struct game_camera *c, float bounds[2], float focus_box[2]);
int camera_focus(struct game_camera *c, float focus_x, float focus_y);
int camera_transform(struct game_camera *c, float x, float y, float out[2]);
void camera_debug(struct game_camera *c);

int camera_contains_area(struct game_camera *c, float x, float y, float w, float h);
int camera_contains_bounds(struct game_camera *c, float x0, float y0, float x1, float y1);

#endif
