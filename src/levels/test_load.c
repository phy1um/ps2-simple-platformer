#include <stdlib.h>
#include <math.h>
#include <p2g/log.h>

#include "../tga.h"
#include "../tiles.h"
#include "levels.h"
#include "fmt.h"
#include "../draw.h"
#include "../game/context.h"

int level_test_load_init(struct gamectx *ctx, struct levelctx *lvl) {
  if (level_load("load.ps2lvl", ctx, lvl)) {
    logerr("load fail");    
    return 1;
  }
  return 0;
}
