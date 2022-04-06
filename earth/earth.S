/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader
 * i.e., the first instructions executed by the processor when boot up
 */
    .section .text.enter
    .global _enter
_enter:
    /* Setup the stack pointer */
    li sp, 0x08003000

    /* First argument of _start is the core ID;
     * SiFive FE310 has only 1 core so the ID is 0 */
    li a0, 0

    /* Second argument of _start is ignored;
       Third argument needs to be initialized as 0 */
    li a2, 0

    /* Enter the C library (in library/libc/libmetal-gloss.a).
     * The C library will do some initialization and then call main() */
    call _start
