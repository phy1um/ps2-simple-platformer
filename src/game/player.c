#include "entity.h"
#include "context.h"
#include "levels/levels.h"

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
static const float GRAVITY = 98;
static const float TERMINAL_VELOCITY = 288.87;
static const float FRICTION = 55.1;
static const float FOOT_OFFSET_Y = 0.001;
static const float FOOT_OFFSET_X = 2.4;
static const float JUMP_SPEED = -120;

#define clampf(n, ma, mi) \
  ((n) > (ma) ? ma : (n) < (mi) ? (mi) : (n))

#define signof(n) \
  ((n)<0 ? -1 : 1)

#define minf(a, b) \
  ((a) < (b) ? (a) : (b))

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
  int pushing_left;
  int pushing_right;
  int has_jump_buffered;
} playerdata;

static int player_draw(struct entity *player, float sx, float sy, struct gamectx *ctx) {
  draw2d_set_colour(0xff, 0xff, 0xff, 0x80);
  draw2d_rect(sx, sy, player->w, player->h);
  return 0;
}

static int player_update(struct entity *player, struct gamectx *ctx, float dt) {
  // horizontal stuff
  float friction = FRICTION*dt;
  float impulse_x = 0;
  if (button_held(DPAD_RIGHT)) {
    impulse_x += 1;
  }
  if (button_held(DPAD_LEFT)) {
    impulse_x -= 1;
  }
  // int jump_impulse = button_held(BUTTON_X);
  playerdata *pd = (playerdata *) player->data;
  if (button_pressed(BUTTON_X)) {
    pd->has_jump_buffered = 1;
  }
  if (button_released(BUTTON_X)) {
    pd->has_jump_buffered = 0;
  }
  

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

  // vertical stuff
  // do gravity
  if (pd->state == FALL || pd->state == JUMP) {
    pd->vy = minf(pd->vy + GRAVITY*dt, TERMINAL_VELOCITY);
    logdbg("apply player gravity: %f", pd->vy);
  } else {
    float foot_x0 = player->x + FOOT_OFFSET_X; 
    float foot_x1 = player->x + player->w - FOOT_OFFSET_X; 
    float foot_y = player->y + player->h + FOOT_OFFSET_Y;
    if (ctx_is_free_point(ctx, foot_x0, foot_y) && ctx_is_free_point(ctx, foot_x1, foot_y)) {
      if (pd->state == STAND) {
        pd->state = FALL;
        pd->vy = 0;
        logdbg("not on ground, falling @ [%f, %f]", player->x, player->y);
      }
    } else {
      if (pd->has_jump_buffered) {
        pd->state = JUMP;
        pd->vy = JUMP_SPEED;
        pd->has_jump_buffered = 0;
      }
    }
  }

  collision_resolve(ctx, player, pd->vx*dt, pd->vy*dt);
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
  playerdata *pd = (playerdata*)tgt->data;
  pd->state = STAND;
  return 0;
}

static void collision_resolve(struct gamectx *ctx, struct entity *e, float dx, float dy) {
  playerdata *pd = (playerdata *)e->data;
  float tgt_x = e->x + dx;
  float tgt_y = e->y + dy;

  // resolve horiz collision  
  if (!ctx_is_free_box(ctx, tgt_x, e->y, e->w, e->h)) {
    if (dx > 0) {
      int target_grid_place = (tgt_x+e->w) / GRID_SIZE; 
      tgt_x = (target_grid_place * GRID_SIZE) - e->w - 0.001;
      pd->pushing_right = 1;
      pd->vx = 0;
      
    } else if (dx < 0) {
      int target_grid_place = tgt_x / GRID_SIZE;
      tgt_x = (target_grid_place+1) * GRID_SIZE;
      pd->pushing_left = 1;
      pd->vx = 0;
    } 
  }

  // resolve vertical collision
  if (!ctx_is_free_box(ctx, tgt_x, tgt_y, e->w, e->h)) {
    if (dy > 0) {
      int target_grid_place = (tgt_y+e->h) / GRID_SIZE; 
      tgt_y = (target_grid_place * GRID_SIZE) - e->h - (FOOT_OFFSET_Y*0.5);
      if (pd->state != STAND) {
        logdbg("hit the ground @ [%f, %f]", tgt_x, tgt_y);
      }
      pd->state = STAND;
      pd->vy = 0;
    } else if (dy < 0) {
      int target_grid_place = tgt_y / GRID_SIZE; 
      tgt_y = (target_grid_place+1) * GRID_SIZE;
      if (pd->state == JUMP) {
        pd->state = FALL;
      }
      pd->vy = 0;
    }
  }
  
  e->x = tgt_x;
  e->y = tgt_y;
  // info("resolve: [%f, %f]", tgt_y, tgt_y);
}
