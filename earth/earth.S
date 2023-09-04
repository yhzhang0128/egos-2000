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
    .global _enter, trap_entry_qemu
_enter:
    /* Disable machine interrupt */
    li t0, 0x8
    csrc mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x80003f80
    call main

trap_entry_qemu:
    csrw mscratch, t0

    /* Set mstatus.MPRV in order to use virtual addresses */
    /* If mstatus.MPP is user mode, set it to supervisor mode */
    li t0, 0x20800
    csrs mstatus, t0

    csrr t0, mscratch

    /* Jump to trap_entry_arty() without modifying any registers */
    beq a0, a0, trap_entry_arty
