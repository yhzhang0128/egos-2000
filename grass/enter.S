/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the stack pointer and call the main function of
 * the grass kernel
 */

        .section .text
        .global _enter
_enter:
        li sp,0x08008000
        addi sp,sp,-4
        jal main
        jal _enter
