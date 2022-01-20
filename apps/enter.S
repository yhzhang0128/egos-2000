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
        li sp,0x8000c000
        addi sp,sp,-64
        jal main
        jal _stop

_stop:
        jal _stop
