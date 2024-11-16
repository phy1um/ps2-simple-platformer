#include <p2g/log.h>
#include <p2g/pad.h>
#include "menu.h"
#include "../game/context.h"

static int *menu_active_ptr = 0;

static int action_nil(struct gamectx *ctx, struct menu_state *menu) {
  return 0;
}

static struct menu_set dbg_menu_1_draw = {
  .entries = {
    {
      .name = "collis",
      .action = action_nil,
    },
    {
      .name = "triggers",
      .action = action_nil,
    },
    {
      .name = "overlays",
      .action = action_nil,
    }
  },
  .cursor = 0,
  .entry_count = 3,
};

static int dbg_draw_submenu(struct gamectx *ctx, struct menu_state *menu) {
  return menu_push(menu, &dbg_menu_1_draw);
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
      .action = dbg_draw_submenu,
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

