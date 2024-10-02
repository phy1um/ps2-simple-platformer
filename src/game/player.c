#include "entity.h"
#include "context.h"

#include <stdlib.h>
#include <math.h>

#include <p2g/ps2draw.h>
#include <p2g/pad.h>
#include <p2g/log.h>


static const float EPSILON = 0.0001;
static const float WIDTH = 9.;
static const float HEIGHT= 16.;
static const float HSPEED = 88.72;
static const float GROUND_ACCEL = 37.2;
static const float GRAVITY = 9.8;
static const float TERMINAL_VELOCITY = 40.2;
static const float FRICTION = 55.1;

#define clampf(n, ma, mi) \
  ((n) > (ma) ? ma : (n) < (mi) ? mi : n)

#define signof(n) \
  ((n)<0 ? -1 : 1)

enum pstate {
  STAND,
  FALL,
  JUMP,
};

static void collision_resolve(struct gamectx *ctx, struct entity *e, float dx, float dy);

typedef struct playerdata {
  float vx; 
  float vy; 
  enum pstate state;
} playerdata;

static int player_draw(struct entity *player, struct gamectx *ctx) {
  draw2d_set_colour(0xff, 0xff, 0xff, 0x80);
  draw2d_rect(player->x, player->y, player->w, player->h);
  return 0;
}

static int player_update(struct entity *player, struct gamectx *ctx, float dt) {
  float friction = FRICTION*dt;
  float impulse_x = 0;
  if (button_held(DPAD_RIGHT)) {
    impulse_x += 1;
  }
  if (button_held(DPAD_LEFT)) {
    impulse_x -= 1;
  }
  playerdata *pd = (playerdata *) player->data;

  if (fabs(impulse_x) > EPSILON) {
    if (fabs(pd->vx) > EPSILON && (signof(impulse_x) != signof(pd->vx))) {
      logdbg("signs: impulse_x = %d, pd->vx = %d", signof(impulse_x), signof(pd->vx));
      logdbg("dir mismatch: impulse=%f, vx=%f", impulse_x, pd->vx);
      friction *= 2.89; 
    } else {
      friction = 0.;
    }
  }
  pd->vx = clampf(pd->vx+impulse_x*GROUND_ACCEL*dt, HSPEED, -HSPEED);

  float dir = signof(pd->vx);
  float new_vx = fabs(pd->vx) - friction;
  if (new_vx < EPSILON) {
    new_vx = 0.;
  }
  pd->vx = dir*new_vx;

  collision_resolve(ctx, player, pd->vx*dt, 0);
  return 0;
}

int player_new(struct entity *tgt, float x, float y) {
  tgt->active = 1;
  tgt->x = x;
  tgt->y = y;
  tgt->w = WIDTH;
  tgt->h = HEIGHT;
  tgt->draw = player_draw;
  tgt->update = player_update;
  tgt->data = calloc(1, sizeof(playerdata));
  return 0;
}

static void collision_resolve(struct gamectx *ctx, struct entity *e, float dx, float dy) {
  float tgt_x = e->x + dx;
  float tgt_y = e->y + dy;

  // resolve horiz collision  
  if (!ctx_is_free_box(ctx, tgt_x, e->y, e->w, e->h)) {
    if (dx > 0) {
      int target_grid_place = (tgt_x+e->w) / ctx->collision.grid_size; 
      tgt_x = (target_grid_place * ctx->collision.grid_size) - e->w - 0.000001;
    } else if (dx < 0) {
      int target_grid_place = tgt_x / ctx->collision.grid_size;
      tgt_x = (target_grid_place+1) * ctx->collision.grid_size;
    } 
  }

  
  e->x = tgt_x;
  e->y = tgt_y;
  // info("resolve: [%f, %f]", tgt_y, tgt_y);
}
