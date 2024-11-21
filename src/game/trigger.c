#include <p2g/log.h>

#include "trigger.h"
#include "task.h"
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
      return task_submit(TASK_LOAD_LEVEL, a->arg);
    default:
      logerr("invalid event trigger: %d", a->kind);
      return 1;
  }
}
