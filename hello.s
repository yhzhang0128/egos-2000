/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the hello-world program
 */
    .section .text.enter
    .global _start

_start:
    li sp, 0x80400000
    call main
_end:
    call _end
