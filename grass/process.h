#pragma once

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
