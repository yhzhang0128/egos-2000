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
    .global earth_entry, trap_from_M_mode, trap_from_S_mode

earth_entry:
    /* Disable machine interrupt */
    li t0, 0x8
    csrc mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x80003f80
    call main

.macro TRAP_START
    /* Write User SP into MScratch */
    csrw mscratch, sp
    /* Adjust SP to top of Kernel Stack */
    lui sp, 0x80004
    addi sp, sp, -0x80
    /* Swap t0 and User SP */
    csrrw t0, mscratch, t0
    addi t0, t0, -116
    /* Save RA of Interrupted Procedure */
    sw ra, 108(t0)
    /* Save all User Argument Registers */
    sw a7, 104(t0)
    sw a6, 100(t0)
    sw a5, 96(t0)
    sw a4, 92(t0)
    sw a3, 88(t0)
    sw a2, 84(t0)
    sw a1, 80(t0)
    sw a0, 76(t0)
    /* Save all User Temporary Registers */
    sw t6, 72(t0)
    sw t5, 68(t0)
    sw t4, 64(t0)
    sw t3, 60(t0)
    sw t2, 56(t0)
    sw t1, 52(t0)
    /* Save all User Saved Registers */
    sw s11,44(t0)
    sw s10,40(t0)
    sw s9, 36(t0)
    sw s8, 32(t0)
    sw s7, 28(t0)
    sw s6, 24(t0)
    sw s5, 20(t0)
    sw s4, 16(t0)
    sw s3, 12(t0)
    sw s2, 8(t0)
    sw s1, 4(t0)
    sw s0, 0(t0)
    /* Write Original User t0 Value on User Stack */
    csrr t1, mscratch
    sw t1, 48(t0)
    /* Write Updated User SP into MScratch */
    csrw mscratch, t0
.endm

trap_from_S_mode:
    csrw mscratch, t0

    /* Set mstatus.MPRV to enable page table translation in M mode */
    /* If mstatus.MPP is U mode, set to S mode for kernel privilege */
    li t0, 0x20800
    csrs mstatus, t0

    /* Jump to trap_entry_start() without modifying any registers */
    csrr t0, mscratch

trap_from_M_mode:
    TRAP_START
    j trap_entry
