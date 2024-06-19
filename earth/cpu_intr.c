/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: interrupt and exception handling
 */

#include "egos.h"

static ulonglong mtime_get() {
    uint low, high;
    do {
        high = REGW(0x200BFF8, 4);
        low  = REGW(0x200BFF8, 0);
    }  while ( REGW(0x200BFF8, 4) != high );

    return (((ulonglong)high) << 32) | low;
}

static int mtimecmp_set(ulonglong time, uint core_id) {
    REGW(0x2004000, core_id * 8 + 4) = 0xFFFFFFFF;
    REGW(0x2004000, core_id * 8 + 0) = (uint)time;
    REGW(0x2004000, core_id * 8 + 4) = (uint)(time >> 32);

    return 0;
}

static uint QUANTUM;
int timer_reset(uint core_id) { return mtimecmp_set(mtime_get() + QUANTUM, core_id); }

/* Both trap functions are defined in earth.S */
void trap_from_M_mode();
void trap_from_S_mode();

void intr_init(uint core_id) {
    QUANTUM = (earth->platform == ARTY)? 5000 : 500000;
    mtimecmp_set(0x0FFFFFFFFFFFFFFFUL, core_id);

    earth->timer_reset = timer_reset;

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
