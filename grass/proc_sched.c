/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: process switching, communication and termination
 * system calls are basically inter-process communication
 */


#include "egos.h"
#include "grass.h"
#include <string.h>

static void proc_yield();
static void proc_syscall();
static void (*kernel_entry)();

struct process proc_set[MAX_NPROCESS];
int proc_curr_idx;
#define curr_pid  PID(proc_curr_idx)

void ctx_entry() {
    kernel_entry();
    /* switch back to user application */
    void* tmp;
    ctx_switch(&tmp, proc_set[proc_curr_idx].sp);
}

void intr_entry(int id) {
    int mepc;
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    if (mepc < VADDR_START) {
        /* IO may be busy; do not interrupt */
        timer_reset();
        return;
    }

    switch(id) {
    case INTR_ID_TMR:
        kernel_entry = proc_yield;
        ctx_start(&proc_set[proc_curr_idx].sp, (void*)KERNEL_STACK_TOP);
        break;
    case INTR_ID_SOFT:
        kernel_entry = proc_syscall;
        ctx_start(&proc_set[proc_curr_idx].sp, (void*)KERNEL_STACK_TOP);
        break;
    default:
        FATAL("Got unknown interrupt #%d", id);
    }
}

static void proc_yield() {
    int proc_next_idx = -1;
    for (int i = 1; i <= MAX_NPROCESS; i++) {
        int tmp_next = (proc_curr_idx + i) % MAX_NPROCESS;
        if (proc_set[tmp_next].status == PROC_READY ||
            proc_set[tmp_next].status == PROC_RUNNING ||
            proc_set[tmp_next].status == PROC_RUNNABLE) {
            proc_next_idx = tmp_next;
            break;
        }
    }

    if (proc_next_idx == -1)
        FATAL("proc_yield: no more runnable process");

    if (proc_next_idx == proc_curr_idx) {
        timer_reset();
        return;
    }

    int next_pid = PID(proc_next_idx);
    int next_status = proc_set[proc_next_idx].status;
    int curr_status = proc_set[proc_curr_idx].status; 

    if (curr_status == PROC_RUNNING)
        proc_set_runnable(curr_pid);
    proc_set_running(next_pid);
    earth->mmu_switch(next_pid);
    proc_curr_idx = proc_next_idx;

    if (next_status == PROC_READY) {
        timer_reset();
        __asm__ volatile("csrw mepc, %0" ::"r"(VADDR_START));
        __asm__ volatile("mret");
    } else if (next_status == PROC_RUNNABLE) {
        timer_reset();
        void* tmp;
        ctx_switch(&tmp, proc_set[proc_curr_idx].sp);
    }

    FATAL("Reach the end of proc_yield without switching to any process");
}


static void proc_send(struct syscall *sc);
static void proc_recv(struct syscall *sc);

static void proc_syscall() {
    /* software interrupt for system call */
    struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;
    int type = sc->type;
    sc->type = SYS_UNUSED;
    *((int*)RISCV_CLINT0_MSIP_BASE) = 0;

    INFO("Got system call #%d", type);
    switch (type) {
    case SYS_RECV:
        proc_recv(sc);
        break;
    case SYS_SEND:
        proc_send(sc);
        break;
    case SYS_EXIT:
        FATAL("proc_syscall: exit not implemented");
    }
}

static void proc_send(struct syscall *sc) {
    sc->retval = 0;
    sc->payload.msg.sender = curr_pid;
    int receiver = sc->payload.msg.receiver;

    int receiver_idx = -1;
    for (int i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].pid == receiver) {
            receiver_idx = i;
            break;
        }
    }

    if (receiver_idx == -1) {
        sc->retval = -1;
        return;
    }

    if (proc_set[receiver_idx].status != PROC_WAIT_TO_RECV) {
        proc_set[proc_curr_idx].status = PROC_WAIT_TO_SEND;
        proc_set[proc_curr_idx].receiver_pid = receiver;
        proc_yield();
    } else {
        struct sys_msg tmp;
        memcpy(&tmp, &sc->payload.msg, SYSCALL_MSG_LEN);
        earth->mmu_switch(receiver);

        memcpy(&sc->payload.msg, &tmp, SYSCALL_MSG_LEN);
        earth->mmu_switch(curr_pid);

        proc_set_runnable(receiver);
    }
}

static void proc_recv(struct syscall *sc) {
    sc->retval = 0;
    sc->payload.msg.sender = 0;
    sc->payload.msg.receiver = curr_pid;

    int sender = -1;
    for (int i = 0; i < MAX_NPROCESS; i++) {
        if (proc_set[i].status == PROC_WAIT_TO_SEND &&
            proc_set[i].receiver_pid == curr_pid) {
            sender = proc_set[i].pid;
            break;
        }
    }

    if (sender == -1) {
        proc_set[proc_curr_idx].status = PROC_WAIT_TO_RECV;
        INFO("proc_recv: set pid=%d to status=%d", proc_set[proc_curr_idx].pid, proc_set[proc_curr_idx].status);
        proc_yield();
    } else {
        sc->payload.msg.sender = sender;

        struct sys_msg tmp;
        earth->mmu_switch(sender);
        memcpy(&tmp, &sc->payload.msg, SYSCALL_MSG_LEN);

        earth->mmu_switch(curr_pid);
        memcpy(&sc->payload.msg, &tmp, SYSCALL_MSG_LEN);

        proc_set_runnable(sender);
    }
}
