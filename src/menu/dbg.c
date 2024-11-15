#include <p2g/log.h>
#include <p2g/pad.h>
#include "menu.h"
#include "../game/context.h"

static int *menu_active_ptr = 0;

static int toggle_debug_draw(struct gamectx *ctx, struct menu_state *menu) {
  logdbg("toggle debug draw :)");
  return 0;
}

static int reload(struct gamectx *ctx, struct menu_state *menu) {
  return ctx_reload(ctx);
}

static int printstats(struct gamectx *ctx, struct menu_state *menu) {
  return ctx_print_stats(ctx);
}

static int close(struct gamectx *ctx, struct menu_state *menu) {
  if (menu_active_ptr) {
    *menu_active_ptr = 0;
  }
  return 0;
}

static struct menu_set dbg_menu_0 = {
  .entries = {
    {
      .name = "dbg draw",
      .action = toggle_debug_draw,
    },
    {
      .name = "stats",
      .action = printstats,
    },
    {
      .name = "reload",
      .action = reload,
    },
    {
      .name = "close",
      .action = close,
    }
  },
  .cursor = 0,
  .entry_count = 4,
};

void menu_dbg_init(struct menu_state *st, int *active) {
  *active = 1;
  menu_active_ptr = active;
  st->stack_head = 1;
  st->stack[0] = &dbg_menu_0;
  st->stack[0]->cursor = 0;
  return;
}

