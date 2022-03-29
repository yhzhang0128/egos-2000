/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the CPU interrupt/exception interface
 */

#include "egos.h"
#include "earth.h"

static handler_t intr_handler;
static handler_t excp_handler;
static void trap_entry()  __attribute__((interrupt, aligned(128)));

int intr_init() {
    INFO("Use direct mode for CPU interrupt handling");
    INFO("Put the address of trap_entry() to CSR register mtvec");
    __asm__ volatile("csrw mtvec, %0" ::"r"(trap_entry));
        
    return 0;
}

#define MCAUSE_INTR_MASK  0x80000000UL
#define MCAUSE_IDMASK     0x000003FFUL

static void trap_entry() {
    int mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));

    int id = mcause & MCAUSE_IDMASK;
    if (mcause & MCAUSE_INTR_MASK) {
        if (intr_handler != NULL)
            intr_handler(id);
        else
            FATAL("Got interrupt %d but handler not registered", id);
    } else {
        int mepc;
        __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
        if (excp_handler != NULL)
            excp_handler(id);
        else
            FATAL("Got exception %d (mepc=%x) but handler not registered", id, mepc);
    }
}

#define MSTATUS_MIE  0x00000008UL
#define MIE_SW       0x008
#define MIE_TMR      0x080

int intr_enable() {
    int tmp;
    /* Enable global interrupt */
    __asm__ volatile("csrrs %0, mstatus, %1"
                     : "=r"(tmp)
                     : "r"(MSTATUS_MIE));
    /* Enable software interrupt */
    __asm__ volatile("csrrs %0, mie, %1"
                     : "=r"(tmp)
                     : "r"(MIE_SW));
    /* Enable timer interrupt */
    __asm__ volatile("csrrs %0, mie, %1"
                     : "=r"(tmp)
                     : "r"(MIE_TMR));
    return 0;
}

int intr_disable() {
    int tmp;
    /* Disable global interrupt */
    __asm__ volatile("csrrc %0, mstatus, %1"
                     : "=r"(tmp)
                     : "r"(MSTATUS_MIE));
    /* Disable software interrupt */
    __asm__ volatile("csrrc %0, mie, %1"
                     : "=r"(tmp)
                     : "r"(MIE_SW));
    /* Disable timer interrupt */
    __asm__ volatile("csrrc %0, mie, %1"
                     : "=r"(tmp)
                     : "r"(MIE_TMR));
    return 0;
}

int intr_register(handler_t _handler) {
    intr_handler = _handler;
    return 0;
}

int excp_register(handler_t _handler) {
    excp_handler = _handler;
    return 0;
}
