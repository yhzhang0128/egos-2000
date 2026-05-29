#pragma once

#include "egos.h"
#include "syscall/syscall.h"
#include <stdbool.h>

enum proc_status {
    PROC_UNUSED,
    PROC_LOADING,
    PROC_READY,
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_PENDING_SYSCALL
};

struct process {
    int pid;
    struct syscall syscall;
    enum proc_status status;
    uint mepc, saved_registers[32];
    /* Student's code goes here (Preemptive Scheduler | System Call). */
    /* Add new fields for lifecycle statistics, MLFQ or process sleep. */
    // use mtime_get to get diff in time, call twice
    unsigned long long start_time;
    unsigned long long turn_around_time;
    unsigned long long response_time;
    unsigned long long cpu_time;
    unsigned long long num_timer_interrupts;
    unsigned long long last_start_time; // used to calculate cpu_time
    bool has_been_scheduled;

    // final task is to implement MLFQ2

    uint level;
    uint remaining_runtime_on_level;    
    /* Student's code ends here. */
};
#define MAX_NPROCESS 16
#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     10000000         /* 10 seconds */
#define MLFQ_LEVEL_RUNTIME(x) (x + 1) * 100000 /* e.g., 100ms for level 0 */
ulonglong mtime_get();

int proc_alloc();
void proc_free(int);
void proc_set_ready(int);
void proc_set_running(int);
void proc_set_runnable(int);
void proc_set_pending(int);

void mlfq_reset_level();
void mlfq_update_level(struct process* p, ulonglong runtime);
void proc_sleep(int pid, uint usec);
void proc_coresinfo();

extern uint core_to_proc_idx[NCORES];
