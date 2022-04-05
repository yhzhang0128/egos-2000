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
.align 4
    /* Set up a simple trap vector to catch anything that goes wrong early in
     * the boot process. */
    la t0, early_trap_vector
    csrw mtvec, t0

    li sp, 0x08003000

    la ra, __metal_before_start

    li a0, 0
    li a1, 0
    li a2, 0
    call _start

/* The GCC port might not emit a __register_frame_info symbol, which eventually
 * results in a weak undefined reference that eventually causes crash when it
 * is dereference early in boot.  We really shouldn't need to put this here,
 * but to deal with what I think is probably a bug in the linker script I'm
 * going to leave this in for now.  At least it's fairly cheap :) */
.weak __register_frame_info
.global __register_frame_info
.section .text.metal.init.__register_frame_info
__register_frame_info:
    ret

.section .text.metal.init.trapvec
.global early_trap_vector
.align 2
early_trap_vector:
    csrr t0, mcause
    csrr t1, mepc
    csrr t2, mtval
    j early_trap_vector

/*
 * Initialize memories to zero
 * This must be called before setting up any stack(s)
 */
.weak __metal_eccscrub_bit
.weak __metal_before_start
.global __metal_before_start
.type __metal_before_start, @function
__metal_before_start:
    /* Save caller ra */
    mv      s0, ra

    la      t0, __metal_eccscrub_bit
    beqz    t0, skip_scrub

    la      t0, __metal_boot_hart
    csrr    a5, mhartid

    /* Disable machine interrupts to be safe */
    li      a3, 8
    csrc    mstatus, a3

    /* Zero out per hart stack */
    mv      t1, sp
    la      t2, __stack_size
    add     t2, t2, sp
    beq     t1, t2, 1f
    jal     __metal_memory_scrub
1:
    bne     a5, t0, skip_scrub

    /* Zero out data segment */
    la      t1, metal_segment_data_target_start
    la      t2, metal_segment_data_target_end
    beq     t1, t2, 1f
    jal     __metal_memory_scrub
1:
    /* Zero out itim memory */
    la      t1, metal_segment_itim_target_start
    la      t2, metal_segment_itim_target_end
    beq     t1, t2, skip_scrub
    jal     __metal_memory_scrub

skip_scrub:
    /* Restore caller ra */
    mv      ra, s0
    ret

.type __metal_memory_scrub, @function
__metal_memory_scrub:
/* Zero out specified memory regions */
1:
    sw      x0, 0(t1)
    addi    t1, t1, 4
    blt     t1, t2, 1b
    ret

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

