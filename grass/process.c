/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"
extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i <= MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Preemptive Scheduler | System Call). */
            /* Initialization of lifecycle statistics, MLFQ or process sleep. */
            proc_set[i].start_time = mtime_get();
            proc_set[i].turn_around_time = 0;
            proc_set[i].response_time = 0;
            proc_set[i].cpu_time = 0;
            proc_set[i].num_timer_interrupts = 0;
            proc_set[i].has_been_scheduled = false;
            proc_set[i].last_start_time = 0; //initializatino is not same as running
            proc_set[i].level = 0;
            proc_set[i].remaining_runtime_on_level = MLFQ_LEVEL_RUNTIME(0);
            proc_set[i].sleep_until_time = 0;
            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */
    for(int i = 1; i <= MAX_NPROCESS; i++) {
        if(proc_set[i].pid == pid) {
            unsigned long long now = mtime_get();
            if(proc_set[i].last_start_time != 0){
                proc_set[i].cpu_time += now - proc_set[i].last_start_time;
                proc_set[i].last_start_time = 0;
            }
            proc_set[i].turn_around_time = now - proc_set[i].start_time; // why is this so off?
            printf("start_time %llu \n", proc_set[i].start_time);
            printf("now %llu \n", now);
            printf("raw cpu %llu \n", proc_set[i].cpu_time);
            printf("Process %d: response time = %llu ms, turn-around time = %llu ms, CPU time = %llu ms, number of timer interrupts = %d\n",
                   pid, proc_set[i].response_time / 10000, proc_set[i].turn_around_time / 10000,
                   proc_set[i].cpu_time / 10000, proc_set[i].num_timer_interrupts);
            break;
        }
    }
    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user processes. */
        for (uint i = 1; i <= MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
    /* Student's code ends here. */
}

/*
implement MLFQ
in process struct, need to maintain the following:
- level
- remaining runtime on the given level
- when providing `runtime` argument, reuse measurements for CPU time

then, need to modify for loop in proc_yield, to follow Rule 3

call this function in proc_yield, where runtime == updated CPU time?
*/
void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Update the MLFQ-related fields in struct process* p after this
     * process has run on the CPU for another runtime microseconds. */
    // oh the idea is to continue to consume, and continuously move down levels 
    while(runtime > 0){
        if(runtime >= p->remaining_runtime_on_level) {
            runtime -= p->remaining_runtime_on_level;
            if(p->level < MLFQ_NLEVELS - 1) {
                p->level++;
            }
            p->remaining_runtime_on_level = MLFQ_LEVEL_RUNTIME(p->level);
        } else {
            p->remaining_runtime_on_level -= runtime;
            runtime = 0;
        }
        if(p->level == MLFQ_NLEVELS - 1) {
            break; // if we are at the lowest level, we dont need to keep track of remaining runtime
        }
    }
    

    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
        for (uint i = 1; i <= MAX_NPROCESS; i++) {
            if (proc_set[i].pid == GPID_SHELL) {
                proc_set[i].level = 0;
                proc_set[i].remaining_runtime_on_level = MLFQ_LEVEL_RUNTIME(0);
                break;
            }
        }
    }

    static ulonglong MLFQ_last_reset_time = 0; // static variable, has lifetime of the entire program
    ulonglong now = mtime_get();
    if(now - MLFQ_last_reset_time >= MLFQ_RESET_PERIOD) {
        for(uint i = 1; i <= MAX_NPROCESS; i++) {
            if(proc_set[i].status != PROC_UNUSED) {
                proc_set[i].level = 0;
                proc_set[i].remaining_runtime_on_level = MLFQ_LEVEL_RUNTIME(0);
            }
        }
        MLFQ_last_reset_time = now;
    }
    /*
    global vs static
    - global can be accessed across files, static can only be accessed within the file or function it is defined in 
    */
    /* Reset the level of all processes every MLFQ_RESET_PERIOD microseconds. */

    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    unsigned long long now = mtime_get();
    for(int i = 1; i <= MAX_NPROCESS; i++) {
        if(proc_set[i].pid == pid) {
            proc_set[i].sleep_until_time = now + (unsigned long long) usec * 10;
            break;
        }
    }

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */
    for (uint i =0; i < NCORES; i++){
        uint proc_idx = core_to_proc_idx[i];
        int pid = proc_set[proc_idx].pid;
        if(proc_idx == 0) printf("Core %d: idle\n", i+1);
        else printf("Core %d: process %d\n", i+1, pid);
    }
    /* Student's code ends here. */
}
