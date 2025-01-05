#include "../entity.h"
#include "../context.h"

#include <stdlib.h>

#include <p2g/ps2draw.h>
#include <p2g/log.h>



typedef struct edata {
  float timer; 
  int impulse_x;
} edata;

static int draw(struct entity *self, float sx, float sy, struct gamectx *ctx) {
  draw2d_set_colour(200, 30, 60, 0x80);
  draw2d_rect(sx, sy, 32, 32);
  return 0;
}

static int update(struct entity *self, struct gamectx *ctx, float dt) {
  edata *ed = (edata *)self->data;
  ed->timer -= dt;
  if (ed->timer < 0) {
    ed->timer = 1.f;
    ed->impulse_x *= -1;
  }
  self->x += 64*dt*((float)ed->impulse_x);
  return 0;
}

static int init(struct entity *self, void *arg) {
  edata *ed = (edata *)self->data;
  ed->timer = 1.f;
  ed->impulse_x = 1;
  return 0;
}

struct entity_class CLASS_TESTER = {
  .identifier = "TESTER",
  .init = init,
  .draw = draw,
  .update = update,
  .data_size = sizeof(edata),
};
