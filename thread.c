/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: cooperative multithreading and synchronization
 */

#include "thread.h"
#include "print.c"
#include <sys/queue.h>

/* Student's code goes here (Cooperative Threads). */
/* Define the TCB and helper functions (if needed) for multi-threading. */

/* Student's code ends here. */

void thread_init() {
  TCB[0].id = 0;
  TCB[0].status = THREAD_RUNNING;
  TCB[0].stack = NULL;
  current_idx = 0;
  for (int i = 1; i < 32; i++) {
    TCB[i].status = THREAD_UNUSED;
  }
}

int find_thread_slot() {
  for (int i = 1; i < 32; i++) {
    if (TCB[i].status == THREAD_UNUSED) {
      return i;
    }
  }
  return -1;
}

void ctx_entry() {
  TCB[current_idx].status = THREAD_RUNNING;
  TCB[current_idx].entry(TCB[current_idx].arg);
  thread_exit();
}

void thread_create(void (*entry)(void *arg), void *arg) {
  /* Student'scode goes here (Cooperative Threads). */
  int slot = find_thread_slot();
  if (slot == -1) {
    return;
  }
  TCB[slot].status = THREAD_READY;
  TCB[slot].id = slot;
  TCB[slot].entry = entry;
  TCB[slot].arg = arg;
  TCB[slot].stack = malloc(STACK_SIZE);
  int prev_idx = current_idx;
  current_idx = slot;
  TCB[prev_idx].status = THREAD_READY;
  ctx_start(&TCB[prev_idx].sp, TCB[slot].stack + STACK_SIZE);
}

void cleanup_zombies() {
  for (int i = 0; i < 32; i++) {
    if (TCB[i].status == THREAD_ZOMBIE && TCB[i].stack != NULL) {
      free(TCB[i].stack);
      TCB[i].stack = NULL;
      TCB[i].status = THREAD_UNUSED;
    }
  }
}

void thread_yield() {
  int slot = -1;
  if (TCB[current_idx].status != THREAD_WAITING) {
    TCB[current_idx].status = THREAD_READY;
  }
  for (int i = 0; i < 32; i++) {
    if (TCB[i].status == THREAD_READY && i != current_idx) {
      slot = i;
      break;
    }
  }
  if (slot == -1) {
    return;
  }
  int prev = current_idx;
  current_idx = slot;
  ctx_switch(&TCB[prev].sp, TCB[slot].sp);
  printf("yield: resumed as thread %d\n\r", current_idx);
  cleanup_zombies();
}

void thread_exit() {
  TCB[current_idx].status = THREAD_ZOMBIE;
  int slot = -1;
  for (int i = 0; i < 32; i++) {
    if (TCB[i].status == THREAD_READY) {
      slot = i;
      break;
    }
  }
  if (slot == -1) {
    _end();
  }
  int prev = current_idx;
  current_idx = slot;
  ctx_switch(&TCB[prev].sp, TCB[slot].sp);
}

void cv_init(struct cv *condition) {
  condition->count = 0;
  for (int i = 0; i < 32; i++) {
    condition->waiting[i] = -1;
  }
}

void cv_wait(struct cv *condition) {
  if (condition->count == 32) {
    return;
  }
  TCB[current_idx].status = THREAD_WAITING;
  condition->waiting[condition->count] = current_idx;
  condition->count++;
  thread_yield();
}

void cv_signal(struct cv *condition) {
  if (condition->count == 0) {
    return;
  }
  int thread_idx = condition->waiting[condition->count - 1];
  condition->count--;
  TCB[thread_idx].status = THREAD_READY;
}

#define BUF_SIZE 3
void *buffer[BUF_SIZE];
int count = 0;
int head = 0, tail = 0;
struct cv nonempty, nonfull;

void produce(void *arg) {
  while (1) {
    while (count == BUF_SIZE)
      cv_wait(&nonfull);
    printf("Producer %d\n\r", *(int *)arg);
    buffer[tail] = arg;
    tail = (tail + 1) % BUF_SIZE;
    count += 1;
    cv_signal(&nonempty);
  }
}

void consume(void *arg) {
  while (1) {
    while (count == 0)
      cv_wait(&nonempty);
    printf("Consumer %d\n\r", *(int *)arg);
    void *result = buffer[head];
    head = (head + 1) % BUF_SIZE;
    count -= 1;
    cv_signal(&nonfull);
  }
}

int main() {
  thread_init();

  int ID[10];
  for (int i = 0; i < 10; i++)
    ID[i] = i;

  cv_init(&nonempty);
  cv_init(&nonfull);

  for (int i = 0; i < 5; i++)
    thread_create(consume, ID + i);

  for (int i = 0; i < 5; i++)
    thread_create(produce, ID + i);

  printf("main thread exits\n\r");
  thread_exit();
}
