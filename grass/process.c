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

static void intr_handler(int id);
void proc_init() {
    earth->intr_register(intr_handler);
}

static void intr_handler(int id) {
    if (id == INTR_ID_TMR) {
        /* timer interrupt for scheduling */
        timer_reset();
    } else if (id == INTR_ID_SOFT) {
        /* software interrupt for system call */
        struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;
        sc->type = SYS_UNUSED;
        *((int*)RISCV_CLINT0_MSIP_BASE) = 0;

        INFO("Got system call #%d with arg %d", sc->type, sc->args.exit.status);        
    } else {
        FATAL("Got unknown interrupt #%d", id);
    }
}
