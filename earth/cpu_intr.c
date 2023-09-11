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

void trap_entry_vm(); /* This wrapper function is defined in earth.S */
void trap_entry()  __attribute__((interrupt ("machine"), aligned(128)));
void trap_entry() {
    int mcause;
    asm("csrr %0, mcause" : "=r"(mcause));

    int id = mcause & 0x3FF;
    if (mcause & (1 << 31))
        (intr_handler)? intr_handler(id) : FATAL("trap_entry: interrupt handler not registered");
    else
        (excp_handler)? excp_handler(id) : FATAL("trap_entry: exception handler not registered");
}

void intr_init() {
    earth->intr_register = intr_register;
    earth->excp_register = excp_register;

    /* Setup the interrupt/exception entry function */
    if (earth->translation == PAGE_TABLE) {
        asm("csrw mtvec, %0" ::"r"(trap_entry_vm));
        INFO("Use direct mode and put the address of trap_entry_vm() to mtvec");
    } else {
        asm("csrw mtvec, %0" ::"r"(trap_entry));
        INFO("Use direct mode and put the address of trap_entry() to mtvec");
    }

    /* Enable the machine-mode timer and software interrupts */
    int mstatus, mie;
    asm("csrr %0, mie" : "=r"(mie));
    asm("csrw mie, %0" ::"r"(mie | 0x88));
    asm("csrr %0, mstatus" : "=r"(mstatus));
    asm("csrw mstatus, %0" ::"r"(mstatus | 0x88));
}
