#include "entity.h"
#include "context.h"

#include <p2g/ps2draw.h>
#include <p2g/pad.h>

const float PLAYER_WIDTH = 9.;
const float PLAYER_HEIGHT= 16.;

static int player_draw(struct entity *player, struct gamectx *ctx) {
  draw2d_set_colour(0xff, 0xff, 0xff, 0x80);
  draw2d_rect(player->x, player->y, player->w, player->h);
  return 0;
}

static int player_update(struct entity *player, struct gamectx *ctx, float dt) {
  float dx = 0;
  if (button_held(DPAD_RIGHT)) {
    dx += 10.2;
  }
  if (button_held(DPAD_LEFT)) {
    dx -= 10.2;
  }
  player->x += dx*dt;
  return 0;
}

int player_new(struct entity *tgt, float x, float y) {
  tgt->active = 1;
  tgt->x = x;
  tgt->y = y;
  tgt->w = PLAYER_WIDTH;
  tgt->h = PLAYER_HEIGHT;
  tgt->draw = player_draw;
  tgt->update = player_update;
  return 0;
}

