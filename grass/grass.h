#pragma once

#include <stdlib.h>

#include "elf.h"
#include "mmu.h"
#include "print.h"
#include "syscall.h"

struct earth *earth;

#define INTR_ID_TMR     7
#define INTR_ID_SOFT    3
#define RISCV_CLINT0_MSIP_BASE           0x2000000
#define RISCV_CLINT0_MTIME_BASE          0x200bff8
#define RISCV_CLINT0_MTIMECMP_BASE       0x2004000

void timer_init();
long long timer_reset();

enum {
    PROC_UNUSED,
    PROC_READY,     // loaded into memory but haven't started running
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_ZOMBIE
};

struct process{
    int pid;
    int status;
    void* sp;
};

/* interface for kernel process sys_proc */
struct pcb_intf {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);
};

#define MAX_NPROCESS       64
#define KERNEL_STACK_TOP   0x08007f80       //0x08008000 - 128
#define PID(x)             proc_set[x].pid

void proc_init();
int  proc_alloc();
void proc_free(int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);
