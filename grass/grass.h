#pragma once

#include <stdlib.h>

#include "mem.h"
#include "elf.h"
#include "print.h"
#include "syscall.h"

struct earth *earth;

#define INTR_ID_TMR        7
#define INTR_ID_SOFT       3

void timer_init();
long long timer_reset();

#define MAX_NPROCESS       64
#define PID(x)    proc_set[x].pid

enum {
    PROC_UNUSED,
    PROC_READY,       // finish loading but haven't started running
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_WAIT_TO_SEND,
    PROC_WAIT_TO_RECV
};

struct process{
    int pid;
    int status;
    void *sp, *mepc;
    int receiver_pid; // used when status is PROC_WAIT_TO_SEND
};

/* interface for kernel process sys_proc */
struct pcb_intf {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);
};

void proc_init();
int  proc_alloc();
void proc_free(int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);

void ctx_entry(void);
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);
