/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: helper functions for managing processes
 */

#include "egos.h"
#include "process.h"
#include "syscall.h"
#include <string.h>

void intr_entry(int id);

void excp_entry(int id) {
    /* Student's code goes here: */

    /* Kill the process if curr_pid is a user app instead of a grass server */

    /* Student's code ends here. */

    FATAL("excp_entry: kernel got exception %d", id);
}

void proc_init() {
    earth->intr_register(intr_entry);
    earth->excp_register(excp_entry);

    /* Student's code goes here: */

    /* Setup PMP TOR region 0x00000000 - 0x08008000 as r/w/x */

    /* Setup PMP NAPOT region 0x20400000 - 0x20800000 as r/-/x */

    /* Setup PMP NAPOT region 0x20800000 - 0x20C00000 as r/-/- */

    /* Setup PMP NAPOT region 0x80000000 - 0x80004000 as r/w/- */

    /* Student's code ends here. */

    /* The first process is currently running */
    proc_set_running(proc_alloc());
}

static void proc_set_status(int pid, int status) {
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

int proc_alloc() {
    static int proc_nprocs = 0;
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid = ++proc_nprocs;
            proc_set[i].status = PROC_LOADING;
            return proc_nprocs;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    if (pid != -1) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user applications */
        for (int i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
