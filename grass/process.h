#pragma once

#include "elf.h"
#include "disk.h"

enum proc_status {
    PROC_UNUSED,
    PROC_LOADING, /* allocated and wait for loading elf binary */
    PROC_READY,   /* finished loading elf and wait for first running */
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_PENDING_SYSCALL
};

#define SAVED_REGISTER_NUM (32 - 3)
/* zero, gp and tp are the 3 registers not saved */
/* zero is always zero; gp/tp are not used in egos-2000 */
#define SAVED_REGISTER_SIZE SAVED_REGISTER_NUM * 4
#define SAVED_REGISTER_ADDR (void*)(EGOS_STACK_TOP - SAVED_REGISTER_SIZE)

struct process {
    int pid;
    struct syscall syscall;
    enum proc_status status;
    uint mepc, saved_register[SAVED_REGISTER_NUM];

    /* Student's code goes here (Preemptive Scheduler). */

    /* Create data structures that hold scheduling information. */

    /* Student's code ends here. */
};

ulonglong mtime_get();
void core_set_idle(uint);

#define MAX_NPROCESS 16
int proc_alloc();
void proc_free(int);
void proc_set_ready(int);
void proc_set_running(int);
void proc_set_runnable(int);
void proc_set_pending(int);
