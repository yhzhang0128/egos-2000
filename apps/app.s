/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: entry point of applications
 * Initialize stack pointer and call the application main().
 */
    .section .text
    .global app_entry
app_entry:
    /* a0 holds the address of argc; See kernel.c for details. */
    lw a0, 0(a0)
    li sp,0x80400000
    call main
    call exit
