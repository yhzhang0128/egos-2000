/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: initialize stack pointer and call application main()
 */
    .section .text
    .global app_entry
app_entry:
    lw a0, 0(a0) /* a0 holds APPS_ARG, the address of integer argc */
    li sp,0x80800000
    call main
    call exit
