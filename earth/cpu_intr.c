/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of CPU interrupt and exception handling
 */

#include "egos.h"
#include "earth.h"

static void (*intr_handler)(int);
static void (*excp_handler)(int);

void trap_entry() {
    int mepc, mcause;
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrr %0, mcause" : "=r"(mcause));

    int id = mcause & 0x3FF;
    if (mcause & (1 << 31)) {
        (intr_handler != NULL)? intr_handler(id) :
            FATAL("trap_entry: interrupt handler not registered");
    } else {
        (excp_handler != NULL)? excp_handler(id) :
            FATAL("trap_entry: exception handler not registered");
    }
}

int intr_enable() {
    int mstatus, mie;
    asm("csrr %0, mstatus" : "=r"(mstatus));
    asm("csrw mstatus, %0" ::"r"(mstatus | 0x8));
    asm("csrr %0, mie" : "=r"(mie));
    asm("csrw mie, %0" ::"r"(mie | 0x88));

    return 0;
}

int intr_register(void (*_handler)(int)) {
    intr_handler = _handler;
    return 0;
}

int excp_register(void (*_handler)(int)) {
    excp_handler = _handler;
    return 0;
}

void intr_init(struct earth* earth) {
    INFO("Use direct mode and put the address of trap_entry() to mtvec");
    asm("csrw mtvec, %0" ::"r"(trap_entry));

    earth->intr_enable = intr_enable;
    earth->intr_register = intr_register;
    earth->excp_register = excp_register;
}
