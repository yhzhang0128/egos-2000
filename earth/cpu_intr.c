/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: RISC-V interrupt and exception handling
 */

#include "egos.h"

static void (*intr_handler)(int);
static void (*excp_handler)(int);

void trap_entry()  __attribute__((interrupt ("machine"), aligned(128)));
void trap_entry() {
    int mcause;
    asm("csrr %0, mcause" : "=r"(mcause));

    int id = mcause & 0x3FF;
    if (mcause & (1 << 31)) {
        (intr_handler != 0)? intr_handler(id) :
            FATAL("trap_entry: interrupt handler not registered");
    } else {
        (excp_handler != 0)? excp_handler(id) :
            FATAL("trap_entry: exception handler not registered");
    }
}

int intr_enable() {
    int mstatus, mie;
    asm("csrr %0, mstatus" : "=r"(mstatus));
    asm("csrw mstatus, %0" ::"r"(mstatus | 0x8));
    asm("csrr %0, mie" : "=r"(mie));
    /* For now, egos-2000 only uses timer and software interrupts */
    asm("csrw mie, %0" ::"r"(mie | 0x88));
}

int intr_register(void (*_handler)(int)) { intr_handler = _handler; }
int excp_register(void (*_handler)(int)) { excp_handler = _handler; }

void intr_init() {
    INFO("Use direct mode and put the address of trap_entry() to mtvec");
    asm("csrw mtvec, %0" ::"r"(trap_entry));

    earth->intr_enable = intr_enable;
    earth->intr_register = intr_register;
    earth->excp_register = excp_register;
}
