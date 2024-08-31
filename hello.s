/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global _start

_start:
    csrr t0, mhartid
    bnez t0, _end      /* Only core#0 will call main */

    li sp, 0x80400000
    call main
_end:
    call _end
