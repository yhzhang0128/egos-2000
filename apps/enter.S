/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the stack pointer for the application
 */

        .section .text
        .global _enter
_enter:
        li sp,0x8000c000
        addi sp,sp,-0x400  /* 1KB reserved for system call */
        jal main
        jal _stop

_stop:
        jal _stop
