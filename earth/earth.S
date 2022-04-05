/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

/* This code executes before _start, which is contained inside the C library.
 * In embedded systems we want to ensure that _enter, which contains the first
 * code to be executed, can be loaded at a specific address.  To enable this
 * feature we provide the '.text.metal.init.enter' section, which is
 * defined to have the first address being where execution should start. */
.section .text.enter
.global _enter
_enter:
    la gp, __global_pointer$
    csrwi 0x7C1, 0
    call earth_scrub
    li sp, 0x08003000
    li a0, 0
    li a1, 0
    li a2, 0
    call _start

/* Function to zero-scrub specified memory
 * a0 : start address for zero-scrub
 * a1 : size memory region size in bytes
 */
.global metal_mem_scrub
.type metal_mem_scrub, @function
metal_mem_scrub:

    /* Disable machine interrupts,
    restore previous mstatus value at exit */
    li      a3, 8
    csrrc   t1, mstatus, a3

    addi    t0, x0, 4
1:
    blt     a1, t0, 2f
    andi    a2, a0, 3
    beqz    a2, 3f
2:
    sb      x0, 0(a0)
    addi    a0, a0, 1
    addi    a1, a1, -1
    bgtz    a1, 1b
    csrw    mstatus, t1
    ret
3:
    sw      x0, 0(a0)
    addi    a0, a0, 4
    addi    a1, a1, -4
    bgtz    a1, 1b
    csrw    mstatus, t1
    ret

    .section .init
    .global __metal_synchronize_harts
__metal_synchronize_harts:
    ret

    .global metal_shutdown, metal_init_run, metal_fini_run
metal_shutdown:
    j metal_shutdown
metal_init_run:
    ret
metal_fini_run:
    ret

