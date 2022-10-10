#pragma once

#include "elf.h"
#include "disk.h"

#define MAX_NPROCESS       16

struct process{
    int pid;
    int status;
    void *sp, *mepc;
    int receiver_pid; /* used when status is PROC_WAIT_TO_SEND */
};
extern int proc_curr_idx;
extern struct process proc_set[MAX_NPROCESS];
#define curr_pid      proc_set[proc_curr_idx].pid
#define curr_status   proc_set[proc_curr_idx].status

enum {
    PROC_UNUSED,
    PROC_LOADING, /* allocated and wait for loading elf binary */
    PROC_READY,   /* finished loading elf and wait for first running */
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_WAIT_TO_SEND,
    PROC_WAIT_TO_RECV
};

void timer_init();
void timer_reset();

void proc_init();
int  proc_alloc();
void proc_free(int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);

void ctx_entry(void);
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);
