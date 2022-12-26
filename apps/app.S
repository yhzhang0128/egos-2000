/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the stack pointer and call application main()
 */
    .section .text
    .global _enter
_enter:
    lw a0, 0(a0) /* a0 holds APPS_ARG, the address of integer argc */
    li sp,0x80002000
    call main
    call exit
