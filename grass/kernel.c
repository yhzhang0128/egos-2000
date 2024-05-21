/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Kernel ≈ 3 handlers
 *     proc_yield() handles timer interrupt for process scheduling
 *     excp_entry() handles faults such as unauthorized memory access
 *     proc_syscall() handles system calls for inter-process communication
 */


#include "egos.h"
#include "process.h"
#include "syscall.h"
#include <string.h>

void kernel_entry(uint is_interrupt, uint id) {
    /* Save process context */
    asm("csrr %0, mepc" : "=r"(proc_set[proc_curr_idx].mepc));
    memcpy(proc_set[proc_curr_idx].saved_register, SAVED_REGISTER_ADDR, SAVED_REGISTER_SIZE);

    (is_interrupt)? intr_entry(id) : excp_entry(id);

    /* Restore process context */
    asm("csrw mepc, %0" ::"r"(proc_set[proc_curr_idx].mepc));
    memcpy(SAVED_REGISTER_ADDR, proc_set[proc_curr_idx].saved_register, SAVED_REGISTER_SIZE);
}

#define EXCP_ID_ECALL_U    8
#define EXCP_ID_ECALL_M    11

void excp_entry(uint id) {
    /* Student's code goes here (system call and memory exception). */

    /* If id is for system call, handle the system call and return */

    /* Otherwise, kill the process if curr_pid is a user application */

    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

#define INTR_ID_SOFT       3
#define INTR_ID_TIMER      7

static void proc_yield();
static void proc_syscall();

uint proc_curr_idx, waiting = 0, wakeup = 0;
struct process proc_set[MAX_NPROCESS];

void intr_entry(uint id) {
    if (id == INTR_ID_TIMER && curr_pid < GPID_SHELL) {
        /* Do not interrupt kernel processes since IO can be stateful */
        earth->timer_reset();
        return;
    }

    if (earth->tty_recv_intr() && curr_pid >= GPID_USER_START) {
        /* User process killed by ctrl+c interrupt */
        INFO("process %d killed by interrupt", curr_pid);
        proc_set[proc_curr_idx].mepc = (uint)sys_exit;
        return;
    }

    /* Ignore other interrupts for now */
    if (id == INTR_ID_SOFT) proc_syscall();
    proc_yield();
}

static void proc_request()
{
    int orig_idx = proc_curr_idx; // To Retain Scheduling Fairness

    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_REQUESTING){
            proc_curr_idx = i;
            earth->mmu_switch(curr_pid);
            proc_syscall();
        }

    proc_curr_idx = orig_idx;
}

static void proc_yield() {
    /* Run all requests to possibly make process runnable */
    proc_request();
    /* Find the next runnable process */
    int next_idx = -1;
    for (uint i = 1; i <= MAX_NPROCESS; i++) {
        enum proc_status s = proc_set[(proc_curr_idx + i) % MAX_NPROCESS].status;
        if (s == PROC_READY || s == PROC_RUNNING || s == PROC_RUNNABLE) {
            next_idx = (proc_curr_idx + i) % MAX_NPROCESS;
            break;
        }
    }

    if (next_idx == -1) FATAL("proc_yield: no runnable process");
    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Switch to the next runnable process and reset timer */
    proc_curr_idx = next_idx;
    earth->mmu_switch(curr_pid);
    earth->timer_reset();

    /* Student's code goes here (switch privilege level). */

    /* Modify mstatus.MPP to enter machine or user mode during mret
     * depending on whether curr_pid is a grass server or a user app
     */

    /* Student's code ends here. */

    /* Call the entry point for newly created process */
    if (curr_status == PROC_READY) {
        /* Set argc, argv and initial program counter */
        proc_set[proc_curr_idx].saved_register[8] = APPS_ARG;
        proc_set[proc_curr_idx].saved_register[9] = APPS_ARG + 4;
        proc_set[proc_curr_idx].mepc = APPS_ENTRY;
    }

    proc_set_running(curr_pid);
}

static int proc_send(struct syscall *sc) {
    sc->msg.sender = curr_pid;
    int receiver = sc->msg.receiver;
    int orig_idx = proc_curr_idx;

    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == receiver) {
            /* Find the receiver */
            if (proc_set[i].received)
                return -1;
            else {
                /* Copy message from sender to kernel stack */
                struct sys_msg tmp;
                memcpy(&tmp, &sc->msg, sizeof(tmp));

                /* Copy message from kernel stack to receiver */
                earth->mmu_switch(receiver);
                memcpy(&sc->msg, &tmp, sizeof(tmp));

                /* Tell receiver they have received */
                proc_set[i].received = 1;

                /* Switch back to sender */
                proc_curr_idx = orig_idx;
                earth->mmu_switch(curr_pid);
                return 0;
            }
        }
    
    return -2;
}

static int proc_recv(struct syscall *sc) {
    if (proc_set[proc_curr_idx].received) {
        proc_set[proc_curr_idx].received = 0;
        return 0;
    }

    return -1;
}

static void proc_syscall() {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;
    int rc = -1;

    proc_set_requesting(curr_pid); // Enter Requesting Mode

    enum syscall_type type = sc->type;
    sc->retval = 0;
    sc->type = SYS_UNUSED;
    *((int*)MSIP) = 0;

    switch (type) {
    case SYS_RECV:
        rc = proc_recv(sc);
        break;
    case SYS_SEND:
        rc = proc_send(sc);
        break;
    case SYS_WAIT:
        waiting++;
        if (wakeup) rc = waiting = wakeup = 0;
        break;
    case SYS_EXIT:
        proc_free(curr_pid);
        if (waiting) wakeup++;
        break;
    default:
        FATAL("proc_syscall: got unknown syscall type=%d", type);
    }

    if (rc == -1)
        sc->type = type; // Failure, Retry Request
    else
        proc_set_runnable(curr_pid); // Either Success or Error, Move to User Space
    
    sc->retval = rc == 0 ? 0 : -1;
}
