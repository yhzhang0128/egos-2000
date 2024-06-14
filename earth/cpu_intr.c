/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: interrupt and exception handling
 */

#include "egos.h"

/* These are two static variables storing
 * the addresses of the handler functions;
 * Initially, both variables are NULL */
static void (*kernel_entry)(uint, uint);
int kernel_entry_init(void (*new_entry)(uint, uint)) {
    kernel_entry = new_entry;
}

/* Both trap functions are defined in earth.S */
void trap_from_M_mode();
void trap_from_S_mode();

void trap_entry() {
    uint mcause;
    asm("csrr %0, mcause" : "=r"(mcause));
    kernel_entry(mcause & (1 << 31), mcause & 0x3FF);
}

void intr_init() {
    earth->kernel_entry_init = kernel_entry_init;

    /* Setup the interrupt/exception entry function */
    if (earth->translation == PAGE_TABLE) {
        asm("csrw mtvec, %0" ::"r"(trap_from_S_mode));
        INFO("Use direct mode and put the address of trap_entry_S_mode() to mtvec");
    } else {
        asm("csrw mtvec, %0" ::"r"(trap_from_M_mode));
        INFO("Use direct mode and put the address of trap_entry_M_mode() to mtvec");
    }

    /* Enable the machine-mode timer and software interrupts */
    uint mstatus, mie;
    asm("csrr %0, mie" : "=r"(mie));
    asm("csrw mie, %0" ::"r"(mie | 0x88));
    asm("csrr %0, mstatus" : "=r"(mstatus));
    asm("csrw mstatus, %0" ::"r"(mstatus | 0x88));
}
