/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: _enter of grass, context start and context switch
 */
    .section .text
    .global _enter, ctx_start, ctx_switch
_enter:
    li sp,0x80003f80
    call main

ctx_start:
    addi sp,sp,-64
    sw s0,4(sp)       /* Save callee-saved registers */
    sw s1,8(sp)
    sw s2,12(sp)
    sw s3,16(sp)
    sw s4,20(sp)
    sw s5,24(sp)
    sw s6,28(sp)
    sw s7,32(sp)
    sw s8,36(sp)
    sw s9,40(sp)
    sw s10,44(sp)
    sw s11,48(sp)
    sw ra,52(sp)      /* Save return address */
    sw sp,0(a0)       /* Save the current stack pointer */
    mv sp,a1          /* Switch the stack */
    call ctx_entry    /* Call ctx_entry() */

ctx_switch:
    addi sp,sp,-64
    sw s0,4(sp)       /* Save callee-saved registers */
    sw s1,8(sp)
    sw s2,12(sp)
    sw s3,16(sp)
    sw s4,20(sp)
    sw s5,24(sp)
    sw s6,28(sp)
    sw s7,32(sp)
    sw s8,36(sp)
    sw s9,40(sp)
    sw s10,44(sp)
    sw s11,48(sp)
    sw ra,52(sp)      /* Save return address */
    sw sp,0(a0)       /* Save the current stack pointer */
    mv sp,a1          /* Switch the stack */
    lw s0,4(sp)       /* Restore callee-saved registers */
    lw s1,8(sp)
    lw s2,12(sp)
    lw s3,16(sp)
    lw s4,20(sp)
    lw s5,24(sp)
    lw s6,28(sp)
    lw s7,32(sp)
    lw s8,36(sp)
    lw s9,40(sp)
    lw s10,44(sp)
    lw s11,48(sp)
    lw ra,52(sp)      /* Restore return address */
    addi sp,sp,64
    ret
