/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: _enter of grass, ctx_exit
 */
    .section .text
    .global grass_entry, ctx_exit

.macro TRAP_EXIT
    /* Read Back User SP */
    csrr sp, mscratch
    /* Restore RA of Interrupted Procedure */
    lw ra, 108(sp)
    /* Restore all User Argument Registers */
    lw a7, 104(sp)
    lw a6, 100(sp)
    lw a5, 96(sp)
    lw a4, 92(sp)
    lw a3, 88(sp)
    lw a2, 84(sp)
    lw a1, 80(sp)
    lw a0, 76(sp)
    /* Restore all User Temporary Registers */
    lw t6, 72(sp)
    lw t5, 68(sp)
    lw t4, 64(sp)
    lw t3, 60(sp)
    lw t2, 56(sp)
    lw t1, 52(sp)
    lw t0, 48(sp)
    /* Restore all User Saved Registers */
    lw s11,44(sp)
    lw s10,40(sp)
    lw s9, 36(sp)
    lw s8, 32(sp)
    lw s7, 28(sp)
    lw s6, 24(sp)
    lw s5, 20(sp)
    lw s4, 16(sp)
    lw s3, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    /* Adjust User SP Back to Original Position Before Interrupt */
    addi sp, sp, 116
.endm

grass_entry:
    li sp,0x80003f80
    call main

ctx_exit:
    TRAP_EXIT
    mret
