#pragma once

#include "elf.h"
#include "disk.h"

enum proc_status {
    PROC_UNUSED,
    PROC_LOADING, /* allocated and wait for loading elf binary */
    PROC_READY,   /* finished loading elf and wait for first running */
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_REQUESTING
};

#define SAVED_REGISTER_NUM  29
#define SAVED_REGISTER_SIZE SAVED_REGISTER_NUM * sizeof(uint)
#define SAVED_REGISTER_ADDR (void*)(GRASS_STACK_TOP - SAVED_REGISTER_SIZE)

struct process{
    int pid;
    enum proc_status status;
    uint mepc, saved_register[SAVED_REGISTER_NUM];
    int received;
};

#define MAX_NPROCESS  8
extern uint proc_curr_idx;
extern struct process proc_set[MAX_NPROCESS];
#define curr_pid      proc_set[proc_curr_idx].pid
#define curr_status   proc_set[proc_curr_idx].status

void intr_entry(uint);
void excp_entry(uint);
void kernel_entry(uint, uint);

int  proc_alloc();
void proc_free(int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);
void proc_set_requesting (int);
