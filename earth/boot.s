/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global boot_loader, hang, boot_lock, booted_core_cnt

boot_loader:
    la t0, boot_lock          /* Load the address of boot_lock. */
    li t1, 1
    amoswap.w.aq t1, t1, (t0) /* Acquire boot_lock. */
    bnez t1, boot_loader
    li sp, 0x80200000
    call boot

hang:
    call hang

.bss
    boot_lock:       .word 0
    booted_core_cnt: .word 0
