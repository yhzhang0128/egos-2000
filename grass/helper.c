/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: helper functions for managing processes
 */

#include "egos.h"
#include "grass.h"
#include <string.h>

void intr_entry(int id);

extern int proc_curr_idx;
extern struct process proc_set[MAX_NPROCESS];

void proc_init() {
    earth->intr_register(intr_entry);
    memset(proc_set, 0, sizeof(proc_set));

    /* the first process is now running */
    int pid = proc_alloc();
    proc_set_running(pid);
}

int proc_alloc() {
    static int proc_nprocs = 0;
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid = ++proc_nprocs;
            proc_set[i].status = PROC_LOADING;
            return proc_nprocs;
        }

    FATAL("Reach the limit of %d concurrent processes", MAX_NPROCESS);
}

static void proc_set_status(int pid, int status) {
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) {
            proc_set[i].status = status;
            return;
        }
}

void proc_free(int pid) {
    if (pid != -1) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user applications */
        for (int i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START
                && proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
}

void proc_set_ready(int pid) {
    proc_set_status(pid, PROC_READY);
}

void proc_set_running(int pid) {
    proc_set_status(pid, PROC_RUNNING);
}

void proc_set_runnable(int pid) {
    proc_set_status(pid, PROC_RUNNABLE);
}
