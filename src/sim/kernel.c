
#include <stdlib.h>

#include <p2g/log.h>
#include <p2g/core.h>
#include "kernel.h"

void * _gp = (void *)0x1000;

static int32_t head = 1;

int32_t CreateSema(ee_sema_t *sema) {
  return head++;
}

int32_t PollSema(int32_t id) {
  if (id <= 0) {
    p2g_fatal("invalid sema: %d", id);
  }
  return 1;
}

int32_t WaitSema(int32_t id) {
  if (id <= 0) {
    p2g_fatal("invalid sema: %d", id);
  }
  return 1;
}

int32_t SignalSema(int32_t id) {
  if (id <= 0) {
    p2g_fatal("invalid sema: %d", id);
  }
  return 1;
}

int32_t CreateThread(ee_thread_t *t) {
  return head++;
}

int32_t StartThread(int32_t id, void *arg) {
  if (id <= 0) {
    p2g_fatal("invalid thread: %d", id);
  }
  return 1;
}

int32_t iWakeupThread(int32_t id) {
  if (id <= 0) {
    p2g_fatal("invalid thread: %d", id);
  }
  return 1;
}

int32_t iRotateThreadReadyQueue(int) {
  return 1;
}

void ExitHandler() {}

int32_t AddIntcHandler(int, void *, void *) {
  return 1;
}

void EnableIntc(int) {}
int32_t GetThreadId() {
  return head++;
}

int32_t SleepThread() {
  return 1;
}


