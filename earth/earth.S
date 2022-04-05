/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader (i.e., the first few instructions executed)
 */
    .section .text.enter
    .global _enter
_enter:
    /* SiFive processor specific initialization */
    csrwi 0x7C1, 0
    /* Setup the stack pointer */
    li sp, 0x08003000
    /* This is core (hart) #0 of the processor */
    li a0, 0
    /* Enter the C library (in library/libc/libmetal-gloss.a).
     * The C library will do some initialization and call main() */
    call _start
