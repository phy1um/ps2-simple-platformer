
#ifndef SRC_MENU_MENU_H
#define SRC_MENU_MENU_H

#include <stddef.h>
#include "../game/context.h"

struct menu_state;
typedef int (*menu_action_fn)(struct gamectx*, struct menu_state*);

struct menu_entry {
  char name[12];
  menu_action_fn action;
};

#define MENU_SET_SIZE 10
struct menu_set {
  struct menu_entry entries[MENU_SET_SIZE];
  size_t cursor;
  size_t entry_count;
};

#define MENU_STACK_SIZE 10
struct menu_state {
  struct menu_set *stack[MENU_STACK_SIZE];
  size_t stack_head;
};

int menu_up(struct menu_state *st);
int menu_down(struct menu_state *st);
int menu_ok(struct menu_state *st, struct gamectx *ctx);
int menu_back(struct menu_state *st);
int menu_push(struct menu_state *st, struct menu_set *set);
int menu_pop(struct menu_state *st);

int menu_draw(struct menu_state *st, float mx, float my);
int menu_drive_inputs(struct menu_state *st, struct gamectx *ctx);

void menu_dbg_init(struct menu_state *st, int *done);

#endif
