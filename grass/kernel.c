/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: Kernel ≈ 2 handlers
 *     intr_entry() handles timer or keyboard ctrl+c interrupts
 *     excp_entry() handles system calls and faults (e.g., unauthorized memory access)
 */

#include "egos.h"
#include "syscall.h"
#include "process.h"
#include <string.h>

uint core_in_kernel;
uint core_curr_proc[NCORES + 1];
struct process proc_set[MAX_NPROCESS + 1]; /* proc_set[0..MAX_NPROCESS-1] are actual processes */
                                           /* proc_set[MAX_NPROCESS] is a place holder for idle cores */

#define PROC_IDLE (curr_proc_idx == MAX_NPROCESS)
void proc_set_idle(uint core) { core_curr_proc[core] = MAX_NPROCESS; }

#define curr_proc_idx core_curr_proc[core_in_kernel]
#define curr_pid      proc_set[curr_proc_idx].pid
#define curr_status   proc_set[curr_proc_idx].status

static void intr_entry(uint);
static void excp_entry(uint);

void kernel_entry(uint mcause) {
    /* With the kernel lock, only one core can enter this point at any time */
    asm("csrr %0, mhartid" : "=r"(core_in_kernel));

    /* Save process context */
    asm("csrr %0, mepc" : "=r"(proc_set[curr_proc_idx].mepc));
    memcpy(proc_set[curr_proc_idx].saved_register, SAVED_REGISTER_ADDR, SAVED_REGISTER_SIZE);

    uint id = mcause & 0x3FF;
    (mcause & (1 << 31))? intr_entry(id) : excp_entry(id);

    /* Restore process context */
    asm("csrw mepc, %0" ::"r"(proc_set[curr_proc_idx].mepc));
    memcpy(SAVED_REGISTER_ADDR, proc_set[curr_proc_idx].saved_register, SAVED_REGISTER_SIZE);
}

#define INTR_ID_CTRL_C     2
#define INTR_ID_TIMER      7
#define EXCP_ID_ECALL_U    8
#define EXCP_ID_ECALL_M    11
static void proc_yield();
static void proc_try_syscall(struct process *proc);

static void excp_entry(uint id) {
    if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
        proc_set[curr_proc_idx].mepc += 4;
        memcpy(&proc_set[curr_proc_idx].sc, (void *)SYSCALL_ARG, sizeof(struct syscall));
        proc_try_syscall(&proc_set[curr_proc_idx]);
        proc_yield();
        return;
    }
    /* Student's code goes here (system call and memory exception). */

    /* Kill the process if curr_pid is a user application */

    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

static void intr_entry(uint id) {
    if (id == INTR_ID_CTRL_C && curr_pid >= GPID_USER_START) {
        /* User process killed by ctrl+c interrupt */
        INFO("process %d killed by interrupt", curr_pid);
        proc_set[curr_proc_idx].mepc = (uint)sys_exit;
    }

    /* Do not interrupt kernel processes since IO can be stateful */
    if (id == INTR_ID_TIMER && (PROC_IDLE || curr_pid >= GPID_SHELL)) proc_yield();
}

static void proc_yield() {
    /* Set the current process status to RUNNABLE if it was RUNNING */
    if (!PROC_IDLE && curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Find the next process to run */
    int next_idx = MAX_NPROCESS;
    for (uint i = 1; i <= MAX_NPROCESS; i++) {
        struct process *p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
        if (p->status == PROC_PENDING_SYSCALL) {
            proc_try_syscall(p); /* Retry pending system call  */
        }
        if (p->status == PROC_READY || p->status == PROC_RUNNABLE) {
            next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
            break;
        }
    }

    /* Context switch */
    curr_proc_idx = next_idx;
    earth->timer_reset(core_in_kernel);
    if (PROC_IDLE) {
        /* Student's code goes here (multi-core and atomic instruction) */

        /* Release earth->kernel_lock and earth->boot_lock; Call proc_idle
         * with mret; Why not do proc_idle() directly? Think about it. */

        /* Student's code ends here. */
        FATAL("proc_yield: no process to run on core %u", core_in_kernel);
    }
    earth->mmu_switch(curr_pid);
    earth->mmu_flush_cache();

    /* Student's code goes here (PMP, page table translation, and multi-core). */

    /* Modify mstatus.MPP to enter machine or user mode during mret
     * depending on whether curr_pid is a grass server or a user app
     */

    /* Student's code ends here. */

    /* Setup the entry point for newly created processes */
    if (curr_status == PROC_READY) {
        /* Set argc, argv and initial program counter */
        proc_set[curr_proc_idx].saved_register[8] = APPS_ARG;
        proc_set[curr_proc_idx].saved_register[9] = APPS_ARG + 4;
        proc_set[curr_proc_idx].mepc              = APPS_ENTRY;
    }
    proc_set_running(curr_pid);
}

static int proc_try_send(struct process *sender) {
    for (uint i = 0; i < MAX_NPROCESS; i++) {
        struct process *dst = &proc_set[i];
        if (dst->pid == sender->sc.msg.receiver && dst->status != PROC_UNUSED) {
            /* Destination is not receiving, or will not take msg from sender */
            if (! (dst->sc.type == SYS_RECV && !dst->sc.msg.received) ) return -1;
            if (! (dst->sc.msg.sender == GPID_ALL || dst->sc.msg.sender == sender->pid) ) return -1;
            
            dst->sc.msg.sender   = sender->pid;
            dst->sc.msg.received = 1;
            memcpy(dst->sc.msg.content, sender->sc.msg.content, SYSCALL_MSG_LEN);
            return 0;
        }
    }
    FATAL("proc_try_send: process %d sending to unknown process %d", sender->pid, sender->sc.msg.receiver);
}

static int proc_try_recv(struct process *receiver) {
    if (!receiver->sc.msg.received) return -1;
    
    receiver->sc.msg.received = 0;
    earth->mmu_switch(receiver->pid);
    memcpy((void*)SYSCALL_ARG, &receiver->sc, sizeof(struct syscall));
    return 0;
}

static void proc_try_syscall(struct process *proc) {
    int rc;

    switch (proc->sc.type) {
    case SYS_RECV:
        rc = proc_try_recv(proc);
        break;
    case SYS_SEND:
        rc = proc_try_send(proc);
        break;
    default:
        FATAL("proc_try_syscall: got unknown syscall type=%d", proc->sc.type);
    }

    if (rc == 0) {
        proc_set_runnable(proc->pid);
        proc->sc.type = SYS_UNUSED;
    } else {
        proc_set_pending(proc->pid);
    }
}

void proc_coresinfo() {
    /* Student's code goes here (multi-core and atomic instruction) */

    /* Print out the pid of the process running on each core */

    /* Student's code ends here. */
}