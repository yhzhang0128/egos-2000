/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: RISC-V interrupt and exception handling
 */

#include "egos.h"

/* These are two static variables storing
 * the addresses of the handler functions;
 * Initially, both variables are NULL */
static void (*intr_handler)(int);
static void (*excp_handler)(int);

/* Register handler functions by modifying the static variables */
int intr_register(void (*_handler)(int)) { intr_handler = _handler; }
int excp_register(void (*_handler)(int)) { excp_handler = _handler; }

void trap_entry_qemu(); /* This wrapper function is defined in earth.S */
void trap_entry_arty()  __attribute__((interrupt ("machine"), aligned(128)));
void trap_entry_arty() {
    int mcause;
    asm("csrr %0, mcause" : "=r"(mcause));

    int id = mcause & 0x3FF;
    if (mcause & (1 << 31))
        (intr_handler)? intr_handler(id) : FATAL("trap_entry: interrupt handler not registered");
    else
        (excp_handler)? excp_handler(id) : FATAL("trap_entry: exception handler not registered");
}

int intr_enable() {
    int mstatus_val, mie_val;
    asm("csrr %0, mstatus" : "=r"(mstatus_val));
    asm("csrw mstatus, %0" ::"r"(mstatus_val | 0x8));
    asm("csrr %0, mie" : "=r"(mie_val));
    /* For now, egos-2000 only uses timer and software interrupts */
    asm("csrw mie, %0" ::"r"(mie_val | 0x88));
}

void intr_init() {
    if (earth->platform == ARTY) {
        asm("csrw mtvec, %0" ::"r"(trap_entry_arty));
        INFO("Use direct mode and put the address of trap_entry_arty() to mtvec");
    } else {
        asm("csrw mtvec, %0" ::"r"(trap_entry_qemu));
        INFO("Use direct mode and put the address of trap_entry_qemu() to mtvec");
    }

    earth->intr_enable = intr_enable;
    earth->intr_register = intr_register;
    earth->excp_register = excp_register;
}
