#pragma once

#include <stdlib.h>

#include "mem.h"
#include "elf.h"
#include "disk.h"
#include "print.h"
#include "syscall.h"
#include "servers.h"

struct earth *earth;
#define MAX_NPROCESS       16

struct process{
    int pid;
    int status;
    void *sp, *mepc;
    int receiver_pid; // used when status is PROC_WAIT_TO_SEND
};

enum {
    PROC_UNUSED,
    PROC_LOADING,     // allocated but waiting to load elf binary
    PROC_READY,       // finish loading but haven't started running
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_WAIT_TO_SEND,
    PROC_WAIT_TO_RECV
};

/* interface of the process control block (pcb) */
struct pcb_intf {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);
};

long long timer_reset();

int  proc_alloc();
void proc_free(int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);

void ctx_entry(void);
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);
