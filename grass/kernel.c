/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: Kernel ≈ 3 handlers
 *     intr_entry() handles timer interrupt for scheduling
 *     excp_entry() handles faults such as unauthorized memory access
 *     proc_try_syscall() handles system calls for inter-process communication
 */


#include "egos.h"
#include "syscall.h"
#include "process.h"
#include <string.h>

uint core_in_grass;
uint core_curr_proc[NCORES + 1];
struct process proc_set[MAX_NPROCESS];

#define curr_proc_idx core_curr_proc[core_in_grass]
#define curr_pid      proc_set[curr_proc_idx].pid
#define curr_status   proc_set[curr_proc_idx].status

static void intr_entry(uint);
static void excp_entry(uint);

void kernel_entry(uint is_interrupt, uint id) {
    asm("csrr %0, mhartid" : "=r"(core_in_grass));

    /* Save process context */
    /* curr_proc_idx == MAX_NPROCESS marks that core_in_grass is idle and not runnig a process  */
    if (curr_proc_idx != MAX_NPROCESS) {
        asm("csrr %0, mepc" : "=r"(proc_set[curr_proc_idx].mepc));
        memcpy(proc_set[curr_proc_idx].saved_register, SAVED_REGISTER_ADDR, SAVED_REGISTER_SIZE);
    }

    (is_interrupt)? intr_entry(id) : excp_entry(id);

    /* Restore process context */
    asm("csrw mepc, %0" ::"r"(proc_set[curr_proc_idx].mepc));
    memcpy(SAVED_REGISTER_ADDR, proc_set[curr_proc_idx].saved_register, SAVED_REGISTER_SIZE);
}

#define INTR_ID_SOFT       3
#define INTR_ID_TIMER      7
#define EXCP_ID_ECALL_U    8
#define EXCP_ID_ECALL_M    11
static void proc_yield();
static void proc_try_syscall(struct process *proc);

static void excp_entry(uint id) {
    if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
        proc_set[curr_proc_idx].mepc += 4;
        proc_try_syscall(&proc_set[curr_proc_idx]);
        proc_yield();
        return;
    }
    /* Student's code goes here (system call and memory exception). */

    /* If id is for system call, handle the system call and return */

    /* Otherwise, kill the process if curr_pid is a user application */

    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

static void intr_entry(uint id) {
    if (earth->tty_recv_intr() && curr_pid >= GPID_USER_START) {
        /* User process killed by ctrl+c interrupt */
        INFO("process %d killed by interrupt", curr_pid);
        proc_set[curr_proc_idx].mepc = (uint)sys_exit;
    }

    /* Do not interrupt kernel processes since IO can be stateful */
    if (id == INTR_ID_TIMER && curr_pid >= GPID_SHELL) proc_yield();
}

static void proc_yield() {
    /* Find the next runnable process */
    int next_idx = -1;
    for (uint i = 1; i <= MAX_NPROCESS; i++) {
        struct process *p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
        if (p->status == PROC_PENDING_SYSCALL) {
            earth->mmu_switch(p->pid);
            proc_try_syscall(p); /* Retry pending system call  */
        }
        if (p->status == PROC_READY || p->status == PROC_RUNNING || p->status == PROC_RUNNABLE) {
            next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
            break;
        }
    }

    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);
    earth->timer_reset(core_in_grass);
    if (next_idx == -1) {
        /* Student's code goes here (multi-core and atomic instruction) */

        FATAL("proc_yield: no process to run on core %u", core_in_grass);

        /* Student's code ends here. */
    }
    curr_proc_idx = next_idx;
    earth->mmu_switch(curr_pid);

    /* Student's code goes here (switch privilege level). */

    /* Modify mstatus.MPP to enter machine or user mode during mret
     * depending on whether curr_pid is a grass server or a user app
     */

    /* Student's code ends here. */

    /* Call the entry point for newly created process */
    if (curr_status == PROC_READY) {
        /* Set argc, argv and initial program counter */
        proc_set[curr_proc_idx].saved_register[8] = APPS_ARG;
        proc_set[curr_proc_idx].saved_register[9] = APPS_ARG + 4;
        proc_set[curr_proc_idx].mepc = APPS_ENTRY;
    }

    proc_set_running(curr_pid);
}

struct pending_ipc *msg_buffer = (void*)(APPS_STACK_TOP + sizeof(struct grass));

static int proc_try_send(struct syscall *sc, struct process *sender) {
    if (msg_buffer->in_use == 1) return -1;
    
    for (uint i = 0; i < MAX_NPROCESS; i++) {
        struct process dst = proc_set[i];
        if (dst.pid == sc->msg.receiver && dst.status != PROC_UNUSED) {
            /* Destination is not receiving, or will not take msg from sender */
            if (! (dst.status == PROC_PENDING_SYSCALL && dst.pending_syscall == SYS_RECV) )   return -1;
            if (! (dst.receive_from == GPID_ALL || dst.receive_from == sender->pid) ) return -1;
            
            msg_buffer->in_use = 1;
            msg_buffer->sender = sender->pid;
            msg_buffer->receiver = sc->msg.receiver;

            memcpy(msg_buffer->msg, sc->msg.content, sizeof(sc->msg.content));
            return 0;
        }
    }
    FATAL("proc_try_send: process %d sending to unknown process %d", sender->pid, sc->msg.receiver);
}

static int proc_try_recv(struct syscall *sc, struct process *receiver) {
    receiver->receive_from = sc->msg.sender;
    
    if (msg_buffer->in_use == 0 || msg_buffer->receiver != receiver->pid) return -1;

    msg_buffer->in_use = 0;
    sc->msg.sender = msg_buffer->sender;
    memcpy(sc->msg.content, msg_buffer->msg, sizeof(sc->msg.content));
    return 0;
}

static void proc_try_syscall(struct process *proc) {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;

    int rc;
    switch (sc->type) {
    case SYS_RECV:
        rc = proc_try_recv(sc, proc);
        break;
    case SYS_SEND:
        rc = proc_try_send(sc, proc);
        break;
    default:
        FATAL("proc_try_syscall: got unknown syscall type=%d", sc->type);
    }

    if (rc == 0) {
        proc_set_runnable(proc->pid);
    } else {
        proc_set_pending(proc->pid);
        proc->pending_syscall = sc->type;
    }
}
