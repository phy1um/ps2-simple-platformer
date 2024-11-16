#include <string.h>

#include <p2g/log.h>
#include <p2g/ps2draw.h>
#include <p2g/pad.h>

#include "menu.h" 

static struct menu_set * get_active_set(struct menu_state *st) {
  if (st->stack_head > MENU_STACK_SIZE) {
    logerr("menu stack overflow: %zu", st->stack_head);
    return 0;
  }
  return st->stack[st->stack_head-1];
}

static struct menu_entry * get_entry(struct menu_set *set, size_t cursor) {
  if (cursor >= MENU_SET_SIZE) {
    logerr("menu entry oob: %zu (/%zu)", cursor, MENU_SET_SIZE);
    return 0;
  }
  return &set->entries[cursor];
}

int menu_up(struct menu_state *st) {
  struct menu_set *set = get_active_set(st);
  if (!set) {
    return 1;
  }
  size_t cursor;
  if (set->cursor == 0) {
    cursor = 0;
  } else {
    cursor = set->cursor - 1;
  }
  struct menu_entry *up = get_entry(set, cursor);  
  if (!up) {
    logerr("nil up entry");
    return 1;
  }
  if (strlen(up->name) != 0) {
    // only update cursor if this entry has a name
    set->cursor = cursor;
  }
  return 0;
}

int menu_down(struct menu_state *st) {
  struct menu_set *set = get_active_set(st);
  if (!set) {
    return 1;
  }
  size_t cursor;
  if (set->cursor >= MENU_SET_SIZE - 1) {
    cursor = MENU_SET_SIZE - 1;
  } else {
    cursor = set->cursor + 1;
  }
  struct menu_entry *down = get_entry(set, cursor);  
  if (!down) {
    logerr("nil down entry");
    return 1;
  }
  if (strlen(down->name) != 0) {
    // only update cursor if this entry has a name
    set->cursor = cursor;
  }
  return 0;
}

int menu_ok(struct menu_state *st, struct gamectx *ctx) {
  struct menu_set *set = get_active_set(st);
  if (!set) {
    return 1;
  }
  struct menu_entry *sel = get_entry(set, set->cursor);  
  if (strlen(sel->name) != 0 && sel->action != 0) {
    return sel->action(ctx, st);
  }
  logerr("menu OK on NIL entry: stack @ %zu, cursor @ %zu", st->stack_head, set->cursor);
  return 1;
}

int menu_back(struct menu_state *st) {
  return menu_pop(st);
}

int menu_push(struct menu_state *st, struct menu_set *set) {
  if (st->stack_head >= MENU_STACK_SIZE) {
    logerr("menu stack overflow: %zu", st->stack_head);
    return 1;
  }
  st->stack[st->stack_head] = set;
  st->stack_head += 1; 
  return 0;
}

int menu_pop(struct menu_state *st) {
  if (st->stack_head == 1) {
    logerr("menu stack underflow");
    return 0;
  }
  st->stack_head -= 1; 
  return 0;
}

static const float MENU_TEXT_SCALE = 2.f;
static const float MENU_COL_W = 14*MENU_TEXT_SCALE;
int menu_draw(struct menu_state *st, struct ee_font *fnt, float mx, float my) {
  if (!st) {
    logerr("draw null menu");
    return 1;
  }

  // logdbg("menu draw start"); 
  for(size_t i = 0; i < st->stack_head; i++) {
    float lhs = mx + (MENU_COL_W*fnt->char_dims[0]*i) + (4*i);
    struct menu_set *set = st->stack[i];
    if (!set) {
      logerr("draw menu: invalid set %zu", i);
      return 1;
    }
    float height = set->entry_count * 18;
    draw2d_set_colour(0, 0, 0, 0x80);
    draw2d_rect(lhs, my, MENU_COL_W*fnt->char_dims[0], height*MENU_TEXT_SCALE);
    for (size_t c = 0; c < set->entry_count; c++) {
      struct menu_entry *e = get_entry(set, c);
      if (!e) {
        logerr("draw menu: invalid entry set=%zu c=%zu", i, c);
        return 1;
      }
      draw2d_set_colour(0x80, 0x80, 0x80, 0x80); 
      font_putstr(fnt, e->name, strlen(e->name), lhs+3.2f, my+((fnt->char_dims[1] + 6.f)*c)*MENU_TEXT_SCALE, MENU_TEXT_SCALE);
      if (c == set->cursor) {
        draw2d_rect(lhs+0.5, my+((fnt->char_dims[1]+6.f)*c)*MENU_TEXT_SCALE+(fnt->char_dims[1]*MENU_TEXT_SCALE)/2, 2, 2);
      }
    }
  }
  // logdbg("menu draw done");
  return 0;
}

int menu_drive_inputs(struct menu_state *st, struct gamectx *ctx) {
  if (button_pressed(DPAD_DOWN)) {
    return menu_down(st);
  } else if (button_pressed(DPAD_UP)) {
    return menu_up(st);
  }
  if (button_pressed(BUTTON_X)) {
    return menu_ok(st, ctx);
  } else if (button_pressed(BUTTON_TRIANGLE)) {
    return menu_back(st);
  }
  return 0;
}


