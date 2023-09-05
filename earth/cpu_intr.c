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

void trap_entry_vmem(); /* This wrapper function is defined in earth.S */
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
    if (earth->translation == PAGE_TABLE) {
        asm("csrw mtvec, %0" ::"r"(trap_entry_vmem));
        INFO("Use direct mode and put the address of trap_entry_vmem() to mtvec");
    } else {
        asm("csrw mtvec, %0" ::"r"(trap_entry));
        INFO("Use direct mode and put the address of trap_entry() to mtvec");
    }

    /* Enable machine-mode interrupts without triggering a timer interrupt */
    #define MTIMECMP_ADDR 0x2004000
    *(int*)(MTIMECMP_ADDR + 4) = 0x0FFFFFFF;
    *(int*)(MTIMECMP_ADDR + 0) = 0xFFFFFFFF;

    int mstatus, mie;
    asm("csrr %0, mie" : "=r"(mie));
    asm("csrr %0, mstatus" : "=r"(mstatus));
    /* For now, egos-2000 only uses timer and software interrupts */
    asm("csrw mie, %0" ::"r"(mie | 0x88));
    asm("csrw mstatus, %0" ::"r"(mstatus | 0x8));

    earth->intr_register = intr_register;
    earth->excp_register = excp_register;
}
