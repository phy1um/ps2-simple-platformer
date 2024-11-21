#ifndef SRC_TASK_H
#define SRC_TASK_H

#define TASK_ARG_LEN 50
#define TASK_STACK_LEN 0x800

enum task_type {
  TASK_LOAD_LEVEL,
};

struct task_async {
  short active;
  short generation;
  enum task_type type;
  char arg[TASK_ARG_LEN];
  char *stack;
  int thread_id;
};

int task_system_init();
int task_submit(enum task_type, const char *arg);

#endif
