#include <p2g/log.h>
#include <string.h>

#include "trigger.h"
#include "../levels/fmt.h"

int trigger_collides_point(struct trigger_area *a, float x, float y) {
  return (x >= a->position[0] 
      && y >= a->position[1]
      && x <= a->position[0] + a->size[0]
      && y <= a->position[1] + a->size[1]);
}

int trigger_event(struct trigger_area *a, struct gamectx *ctx, struct levelctx *lvl) {
  switch(a->kind) {
    case TRIGGER_LOAD_LEVEL:
      const char *arg = a->arg;
      struct levelctx *x = ctx_get_inactive_level(ctx);
      // logdbg("inactive lvl ptr: %p", x);
      if (x->loaded_name == 0 || strcmp(x->loaded_name, arg) != 0) {
        ctx_load_level(ctx, fmt_load_level, arg);
      } else {
        // logdbg("skip load (already loaded): (%s vs %s)", arg, x->loaded_name);
      }
      return 0;
    default:
      logerr("invalid event trigger: %d", a->kind);
      return 1;
  }
}
