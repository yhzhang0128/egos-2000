/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global boot_loader, boot_lock, kernel_lock

boot_loader:
    la t0, boot_lock          /* Load the address of boot_lock */
    li t1, 1
    amoswap.w.aq t1, t1, (t0) /* Acquire boot_lock */
    bnez t1, boot_loader
    /* Student's code goes here (multi-core and atomic instruction) */
    /* Acquire kernel_lock */

    /* Student's code ends here. */
    li sp, 0x80400000
    call boot

.bss
    boot_lock: .word 0
    kernel_lock: .word 0
