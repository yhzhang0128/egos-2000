/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader
 * i.e., the first instructions executed by the CPU when boot up
 */
    .section .image.placeholder
    .section .text.enter
    .global _enter, trap_entry_wrapped
_enter:
    /* Disable machine interrupt */
    li t0, 8
    csrrc t1, mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x80003f80
    call main

trap_entry_wrapped:
    csrw mscratch, t0

    /* Set mstatus.MPRV in order to use virtual addresses */
    li t0, 0x20000
    csrs mstatus, t0

    csrr t0, mscratch

    /* Jump to trap_entry() without modifying any registers */
    beq a0, a0, trap_entry
