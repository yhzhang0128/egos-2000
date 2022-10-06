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
    .global _enter
_enter:
    /* Disable machine interrupt */
    li t0, 8
    csrrc t1, mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x80003f80
    call main
