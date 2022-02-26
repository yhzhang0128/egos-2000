/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: context switch and context start
 */
    .section .text
    .global ctx_start
    .global ctx_switch
ctx_switch:
        addi sp,sp,-64
        sw ra,0(sp)      /* save return address */
        sw s0,4(sp)      /* save callee-saved registers */
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
        sw sp,0(a0)       /* save the current stack pointer */
        mv sp,a1          /* switch the stack */
        lw ra,0(sp)       /* restore return address */
        lw s0,4(sp)       /* restore callee-saved registers */
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
        addi sp,sp,64
        ret

ctx_start:
        addi sp,sp,-64
        sw ra,0(sp)      /* save return address */
        sw s0,4(sp)      /* save callee-saved registers */
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
        sw sp,0(a0)       /* save the current stack pointer */
        mv sp,a1          /* switch the stack */
        jal ctx_entry     /* call ctx_entry() */
