/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: kernel ≈ 2 handlers
 *   intr_entry() handles timer and device interrupts.
 *   excp_entry() handles system calls and faults (e.g., invalid memory access).
 */

#include "process.h"
#include <string.h>

uint core_in_kernel;
uint core_to_proc_idx[NCORES];
struct process proc_set[MAX_NPROCESS + 1];
/* proc_set[MAX_NPROCESS] is a place holder for idle cores. */
void core_set_idle(uint core) { core_to_proc_idx[core] = MAX_NPROCESS; }

#define curr_proc_idx core_to_proc_idx[core_in_kernel]
#define curr_pid      proc_set[curr_proc_idx].pid
#define curr_status   proc_set[curr_proc_idx].status
#define curr_saved    proc_set[curr_proc_idx].saved_registers

static void intr_entry(uint);
static void excp_entry(uint);

void kernel_entry() {
    /* With the kernel lock, only one core can enter this point at any time. */
    asm("csrr %0, mhartid" : "=r"(core_in_kernel));

    /* Save the process context. */
    asm("csrr %0, mepc" : "=r"(proc_set[curr_proc_idx].mepc));
    memcpy(curr_saved, SAVED_REGISTER_ADDR, SAVED_REGISTER_SIZE);

    uint mcause;
    asm("csrr %0, mcause" : "=r"(mcause));
    (mcause & (1 << 31)) ? intr_entry(mcause & 0x3FF) : excp_entry(mcause);

    /* Restore the process context. */
    asm("csrw mepc, %0" ::"r"(proc_set[curr_proc_idx].mepc));
    memcpy(SAVED_REGISTER_ADDR, curr_saved, SAVED_REGISTER_SIZE);
}

#define INTR_ID_TIMER   7
#define EXCP_ID_ECALL_U 8
#define EXCP_ID_ECALL_M 11
static void proc_yield();
static void proc_try_syscall(struct process* proc);

static void excp_entry(uint id) {
    if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
        /* Copy the system call arguments from user space to the kernel. */
        uint syscall_paddr = earth->mmu_translate(curr_pid, SYSCALL_ARG);
        memcpy(&proc_set[curr_proc_idx].syscall, (void*)syscall_paddr,
               sizeof(struct syscall));
        proc_set[curr_proc_idx].syscall.status = PENDING;

        proc_set_pending(curr_pid);
        proc_set[curr_proc_idx].mepc += 4;
        proc_try_syscall(&proc_set[curr_proc_idx]);
        proc_yield();
        return;
    }
    /* Student's code goes here (System Call & Protection | Virtual Memory). */

    /* Kill the current process if curr_pid is a user application. */

    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

static void intr_entry(uint id) {
    if (id != INTR_ID_TIMER) FATAL("excp_entry: kernel got interrupt %d", id);
    /* Student's code goes here (Preemptive Scheduler). */

    /* Update the process lifecycle statistics. */

    /* Student's code ends here. */
    proc_yield();
}

static void proc_yield() {
    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Student's code goes here (Multiple Projects). */

    /* [Preemptive Scheduler]
     * Measure and record lifecycle statistics for the *current* process.
     * Modify the loop below to find the next process to schedule with MLFQ.
     * [System Call & Protection]
     * Do not schedule a process that should still be sleeping at this time. */

    int next_idx = MAX_NPROCESS;
    for (uint i = 1; i <= MAX_NPROCESS; i++) {
        struct process* p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
        if (p->status == PROC_PENDING_SYSCALL) proc_try_syscall(p);

        if (p->status == PROC_READY || p->status == PROC_RUNNABLE) {
            next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
            break;
        }
    }

    if (next_idx < MAX_NPROCESS) {
        /* [Preemptive Scheduler]
         * Measure and record lifecycle statistics for the *next* process.
         * [System Call & Protection | Multicore & Locks]
         * Modify mstatus.MPP to enter machine or user mode after mret. */

    } else {
        /* [Multicore & Locks]
         * Release the kernel lock.
         * [Multicore & Locks | System Call & Protection]
         * Set curr_proc_idx to MAX_NPROCESS; Reset the timer;
         * Enable interrupts by setting the mstatus.MIE bit to 1;
         * Wait for the next interrupt using the wfi instruction. */

        FATAL("proc_yield: no process to run on core %d", core_in_kernel);
    }
    /* Student's code ends here. */

    curr_proc_idx = next_idx;
    earth->mmu_switch(curr_pid);
    earth->mmu_flush_cache();
    if (curr_status == PROC_READY) {
        /* Setup argc, argv and program counter for a newly created process. */
        curr_saved[0]                = APPS_ARG;
        curr_saved[1]                = APPS_ARG + 4;
        proc_set[curr_proc_idx].mepc = APPS_ENTRY;
    }
    proc_set_running(curr_pid);
    earth->timer_reset(core_in_kernel);
}

static void proc_try_send(struct process* sender) {
    for (uint i = 0; i < MAX_NPROCESS; i++) {
        struct process* dst = &proc_set[i];
        if (dst->pid == sender->syscall.receiver &&
            dst->status != PROC_UNUSED) {
            /* Return if dst is not receiving or not taking msg from sender. */
            if (!(dst->syscall.type == SYS_RECV &&
                  dst->syscall.status == PENDING))
                return;
            if (!(dst->syscall.sender == GPID_ALL ||
                  dst->syscall.sender == sender->pid))
                return;

            dst->syscall.status = DONE;
            dst->syscall.sender = sender->pid;
            /* Copy the system call arguments within the kernel PCB. */
            memcpy(dst->syscall.content, sender->syscall.content,
                   SYSCALL_MSG_LEN);
            return;
        }
    }
    FATAL("proc_try_send: unknown receiver pid=%d", sender->syscall.receiver);
}

static void proc_try_recv(struct process* receiver) {
    if (receiver->syscall.status == PENDING) return;

    /* Copy the system call struct from the kernel back to user space. */
    uint syscall_paddr = earth->mmu_translate(receiver->pid, SYSCALL_ARG);
    memcpy((void*)syscall_paddr, &receiver->syscall, sizeof(struct syscall));

    /* Set the receiver and sender back to RUNNABLE. */
    proc_set_runnable(receiver->pid);
    proc_set_runnable(receiver->syscall.sender);
}

static void proc_try_syscall(struct process* proc) {
    switch (proc->syscall.type) {
    case SYS_RECV:
        proc_try_recv(proc);
        break;
    case SYS_SEND:
        proc_try_send(proc);
        break;
    default:
        FATAL("proc_try_syscall: unknown syscall type=%d", proc->syscall.type);
    }
}
