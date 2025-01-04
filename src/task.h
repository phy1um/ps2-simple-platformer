#ifndef SRC_TASK_H
#define SRC_TASK_H

#define TASK_ARG_LEN 50

#ifndef TASK_STACK_SIZE
#define TASK_STACK_SIZE 0x2000
#endif

#ifndef TASK_LIST_SIZE
#define TASK_LIST_SIZE 10
#endif

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
