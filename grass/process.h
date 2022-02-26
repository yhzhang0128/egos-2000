#pragma once

enum {
    PROC_UNUSED,
    PROC_READY,     // loaded into memory but haven't started running
    PROC_RUNNING,
    PROC_RUNNABLE,
};

struct process{
    int pid;
    int status;
};

/* interface for kernel process sys_proc */
struct pcb_intf {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);
};
