#pragma once

enum {
    PROC_UNUSED,
    PROC_RUNNING,
    PROC_RUNNABLE,
};

struct process{
    int pid;
    int status;
};

/* interface for kernel process sys_proc */
struct pcb_intf {
    long long (*timer_reset)();
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_running)(int);
    void (*proc_set_runnable)(int);
};
