
#include <stddef.h>
#include <string.h>
#include <kernel.h>
#include <malloc.h>

#include <p2g/log.h>

#include "task.h"
#include "game/context.h"
#include "levels/fmt.h"

#define TASK_LIST_SIZE 10
static size_t task_list_head = 0;
static struct task_async task_list[TASK_LIST_SIZE];
int task_list_lock = 0;

void load_level_task_fn(void *task_id_p) {
  size_t task_id = (size_t)task_id_p;
  WaitSema(task_list_lock);
  // TODO: maybe need a lock on each task
  if (ctx_load_level(GLOBAL_CTX, fmt_load_level, task_list[task_id].arg)) {
    logerr("level load task failed");
  }
  task_list[task_id].active = 0;
  SignalSema(task_list_lock);
}

int task_system_init() {
  ee_sema_t lock = {
    .init_count = 1,
    .max_count = 1,
  };
  task_list_lock = CreateSema(&lock);
  if (task_list_lock == -1) {
    logerr("task sys init: failed to create sema");
    return 1;
  }
  for (int i = 0; i < TASK_LIST_SIZE; i++) {
    task_list[i].active = 0;
    task_list[i].stack = memalign(64, TASK_STACK_LEN);
  }
  return 0;
}

int task_submit(enum task_type type, const char *arg) {
  WaitSema(task_list_lock);
  for (int i = 0; i < TASK_LIST_SIZE; i++) {
    if (!task_list[i].active) {
      continue;
    }
    if (task_list[i].type == type && strcmp(arg, task_list[i].arg) == 0) {
      // task already in progress :o
      SignalSema(task_list_lock);
      return 0;
    }
  }
  struct task_async *next = &task_list[task_list_head];
  size_t task_id = task_list_head;
  task_list_head = (task_list_head + 1) % TASK_LIST_SIZE;
  if (next->active) {
    logerr("task ring buffer overflow");
    goto err;
  }
  next->active = 1;
  next->generation += 1;
  next->type = type;
  strncpy(next->arg, arg, TASK_ARG_LEN);
  ee_thread_t task_thread = {
    .stack = next->stack,
    .stack_size = TASK_STACK_LEN,
    .gp_reg = &_gp,
    .initial_priority = 15,
  };
  switch (type) {
    case TASK_LOAD_LEVEL:
      task_thread.func = load_level_task_fn;
      break;
    default:
      logerr("unknown task type: %d", type);
      goto err;
  }
  next->thread_id = CreateThread(&task_thread);
  StartThread(next->thread_id, ((void*)task_id));
  SignalSema(task_list_lock);
  return 0;
err:
  SignalSema(task_list_lock);
  return 1;
}

