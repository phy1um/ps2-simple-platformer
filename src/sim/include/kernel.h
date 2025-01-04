#ifndef SRC_SIM_INCLUDE_KERNEL_H
#define SRC_SIM_INCLUDE_KERNEL_H

#include <stdint.h>

extern void * _gp;

typedef struct {
  int init_count;
  int max_count;
} ee_sema_t;

typedef struct {
  void *stack;
  size_t stack_size;
  void *gp_reg;
  int initial_priority;
  void *func;
} ee_thread_t;

int32_t CreateSema(ee_sema_t *sema);
int32_t PollSema(int32_t id);
int32_t WaitSema(int32_t id);
int32_t SignalSema(int32_t id);

int32_t CreateThread(ee_thread_t *t);
int32_t StartThread(int32_t id, void *arg);
int32_t iWakeupThread(int32_t);
int32_t iRotateThreadReadyQueue(int);
void ExitHandler();
int32_t AddIntcHandler(int, void *, void *);
void EnableIntc(int);
int32_t GetThreadId();
int32_t SleepThread();

#endif
