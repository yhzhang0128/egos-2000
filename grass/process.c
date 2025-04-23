/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for managing processes
 */

#include "process.h"

extern uint core_in_kernel;
extern uint core_to_proc_idx[NCORES + 1];
extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    /* Student's code goes here (Preemptive Scheduler | System Call). */

    /* Collect information (e.g., spawning time) for the new process
     * and initialize the MLFQ data structures. Initialize fields of
     * struct process added for process sleep. */

    static uint curr_pid = 0;
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            return curr_pid;
        }

    /* Student's code ends here. */
    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Collect information (e.g., termination time) for process pid,
     * and print out scheduling metrics. Cleanup MLFQ data structures. */

    if (pid != GPID_ALL) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user applications */
        for (uint i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }

    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the struct process of process pid for process sleep. */

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print the pid of the process running on each core; Add this
     * function into the grass interface so that shell can invoke it. */

    /* Student's code ends here. */
}

void mlfq_reset_level() {
    static ulonglong MLFQ_last_reset_time = 0;
    /* Student's code goes here (Preemptive Scheduler). */

    /* Reset the level of all processes every RESET_RATE microseconds. */

    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Update the struct process for process p if p is a user process
     * and it has used the CPU for another runtime microseconds. */

    /* Student's code ends here. */
}