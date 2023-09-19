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
    .global earth_entry, trap_entry_vm
earth_entry:
    /* Disable machine interrupt */
    li t0, 0x8
    csrc mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x80003f80
    call main

trap_entry_vm:
    csrw mscratch, t0

    /* Set mstatus.MPRV to enable page table translation in M mode */
    /* If mstatus.MPP is U mode, set to S mode for kernel privilege */
    li t0, 0x20800
    csrs mstatus, t0

    /* Jump to trap_entry() without modifying any registers */
    csrr t0, mscratch
    j trap_entry
