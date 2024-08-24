/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global boot_loader

boot_loader:
    li t0, 0x80200000
    li t1, 1
    amoswap.w.aq t1, t1, (t0)           /* Acquire earth->boot_lock */
    bnez t1, boot_loader
    /* Student's code goes here (multi-core and atomic instruction) */
    /* Acquire earth->kernel_lock */

    /* Student's code ends here. */
    li sp, 0x80400000
    call boot
