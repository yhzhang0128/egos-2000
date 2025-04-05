/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: entry point of applications
 * Initialize stack pointer and call the application main()
 */
    .section .text
    .global app_entry
app_entry:
    /* a0 holds APPS_ARG, the address of argc (see kernel.c for details) */
    lw a0, 0(a0)
    li sp,0x80800000
    call main
    call exit
