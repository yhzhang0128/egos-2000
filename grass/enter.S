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
        li sp,0x80003f80
        call main
        call sys_exit
_spin:
        j _spin
