#include <string.h>
#include <p2g/log.h>
#include <p2g/pad.h>
#include "camera.h"

int camera_init(struct game_camera *c, float bounds[2], float focus_box[2]) {
  if (!c) {
    logerr("init NULL camera");
    return 1;
  }
  memset(c->position, 0, 2*sizeof(float));
  memcpy(c->bounds, bounds, 2*sizeof(float));
  memcpy(c->focus_box, focus_box, 2*sizeof(float));
  c->scale = 1.f;
  c->blend_colour[0] = 0x80;
  c->blend_colour[1] = 0x80;
  c->blend_colour[2] = 0x80;
  c->blend_colour[3] = 0x80;
  return 0;
}

int camera_focus(struct game_camera *c, float focus_x, float focus_y) {
  if (!c) {
    logerr("focus NULL camera");
    return 1;
  }
  float cx = c->position[0] + c->bounds[0]/2.f;
  float cy = c->position[1] + c->bounds[1]/2.f;
  float fbox_x0 = cx - c->focus_box[0]/2.f;
  float fbox_x1 = cx + c->focus_box[0]/2.f;
  float fbox_y0 = cy - c->focus_box[1]/2.f;
  float fbox_y1 = cy + c->focus_box[1]/2.f;

  int moved = 0;

  if (focus_x < fbox_x0) {
    float dx = fbox_x0 - focus_x;
    //logdbg("focus x: %f < fbox: dx = %f", focus_x, dx);
    c->position[0] -= dx;
    moved = 1;
  } else if (focus_x > fbox_x1) {
    float dx = focus_x - fbox_x1;
    //logdbg("focus x: %f > fbox: dx = %f", focus_x, dx);
    c->position[0] += dx;
    moved = 1;
  }

  if (focus_y < fbox_y0) {
    float dy = fbox_y0 - focus_y;
    c->position[1] -= dy;
    moved = 1;
  } else if (focus_y > fbox_y1) {
    float dy = focus_y - fbox_y1;
    c->position[1] += dy;
    moved = 1;
  }

  if (moved) {
    //logdbg("new focus box: [%f, %f] -> [%f, %f]", fbox_x0, fbox_y0, fbox_x1, fbox_y1);
  }
  return 0;
}


int camera_transform(struct game_camera *c, float x, float y, float out[2]) {
  if (!c) {
    logerr("transform NULL camera");
    return 1;
  }
  out[0] = x - c->position[0];
  out[1] = y - c->position[1];
  return 0;
}

void camera_debug(struct game_camera *c) {
  float cx = c->position[0] + c->bounds[0]/2.f;
  float cy = c->position[1] + c->bounds[1]/2.f;
  float fbox_x0 = cx - c->focus_box[0]/2.f;
  float fbox_x1 = cx + c->focus_box[0]/2.f;
  float fbox_y0 = cy - c->focus_box[1]/2.f;
  float fbox_y1 = cy + c->focus_box[1]/2.f;

  if (button_pressed(BUTTON_TRIANGLE)) {
    info("CAMERA @: [%f, %f]", c->position[0], c->position[1]);
    info("CAMERA FOCUS BOX: [%f, %f] -> [%f, %f]", fbox_x0, fbox_y0, fbox_x1, fbox_y1);
  }
}

int camera_contains_area(struct game_camera *c, float x, float y, float w, float h) {
  return (c->position[0] < x+w 
      && x < c->position[0]+c->bounds[0] 
      && c->position[1] < y+h 
      && y < c->position[1] + c->bounds[1]);
}

int camera_contains_bounds(struct game_camera *c, float x0, float y0, float x1, float y1) {
  float w = (x1 - x0);
  float h = (y1 - y0);
  return camera_contains_area(c, x0, y0, w, h);
}

