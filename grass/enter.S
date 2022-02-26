/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the stack pointer for the grass kernel
 */

        .section .text
        .global _enter
_enter:
        li sp,0x08008000
        addi sp,sp,-128   /* 128 bytes reserved for struct earth */
        jal main
        jal _stop

_stop:
        jal _stop
