/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global _start

_start:
    li sp, 0x80400000
    call main
_end:
    /* wait for interrupt, put CPU core into low-power mode. A good way to finish any bare-metal program. */
    wfi
    call _end
