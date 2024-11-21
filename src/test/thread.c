#include <kernel.h>
#include <timer.h>
#include <timer_alarm.h>
#include <delaythread.h>
#include <stdio.h>

unsigned char thst[0x800] __attribute__((aligned(64)));
static int i = 0;
static s32 main_tid;
static s32 other_tid;

void runthread() {
  while(1) {
    SleepThread();
    i++;
    printf("inc i = %d\n", i);
  }
}

static u64 wakeup(s32 id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value) {
  if (i < 3) {
    iWakeupThread(other_tid);
  } else {
    iTerminateThread(other_tid);
    iWakeupThread(main_tid);
  }
  return 1;
}

int main(int argc, char *argv[]) {
  printf("hello world\n");
  ee_thread_t th = {
    .func = runthread,
    .stack = thst,
    .stack_size = 0x800,
    .gp_reg = &_gp,
    .initial_priority = 5,
  };
  other_tid = CreateThread(&th);
  StartThread(other_tid, NULL);
  main_tid = GetThreadId();
  SetTimerAlarm(Sec2TimerBusClock(1), wakeup, NULL);
  SleepThread();
  // DelayThread(500 * 1000);
  printf("main done i = %d\n", i);
  while(1) {}
  return 0;
}

