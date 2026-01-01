/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the kernel
 * When receiving an interrupt or exception, the CPU sets
 * its program counter to the first instruction of trap_entry.
 */
    .section .text
    .global trap_entry, kernel_lock

trap_entry:
    /* Step1: Acquire the kernel lock (only for multicore).
     * Step2: Switch to the kernel stack.
     * Step3: Save all the registers on the kernel stack.
     * Step4: Call kernel_entry().
     * Step5: Restore all the registers.
     * Step6: Switch back to the process stack.
     * Step7: Release the kernel lock (only for multicore).
     * Step8: Invoke mret, returning to the process context. */

    /* Step1 */
    /* Student's code goes here (Multicore & Locks). */
    /* Acquire the kernel lock and make sure not to modify any registers, */
    /* so you may need to use sscratch just like how Step2 uses mscratch. */

    /* Student's code ends here. */

    /* Step2 */
    csrw mscratch, sp
    li sp, 0x80200000

    /* Step3 */
    addi sp, sp, -128 /* now, sp == EGOS_STACK_TOP-32*4 */
    sw a0,  0(sp)
    sw a1,  4(sp)
    sw a2,  8(sp)
    sw a3,  12(sp)
    sw a4,  16(sp)
    sw a5,  20(sp)
    sw a6,  24(sp)
    sw a7,  28(sp)
    sw t0,  32(sp)
    sw t1,  36(sp)
    sw t2,  40(sp)
    sw t3,  44(sp)
    sw t4,  48(sp)
    sw t5,  52(sp)
    sw t6,  56(sp)
    sw s0,  60(sp)
    sw s1,  64(sp)
    sw s2,  68(sp)
    sw s3,  72(sp)
    sw s4,  76(sp)
    sw s5,  80(sp)
    sw s6,  84(sp)
    sw s7,  88(sp)
    sw s8,  92(sp)
    sw s9,  96(sp)
    sw s10, 100(sp)
    sw s11, 104(sp)
    sw ra,  108(sp)
    sw gp,  112(sp)
    sw tp,  116(sp)
    csrr t0, mscratch /* Step1 has written sp to mscratch */
    sw t0,  120(sp)   /* t0 holds the value of the old sp before trap_entry */

    /* Step4 */
    call kernel_entry

    /* Step5 */
    lw a0,  0(sp)
    lw a1,  4(sp)
    lw a2,  8(sp)
    lw a3,  12(sp)
    lw a4,  16(sp)
    lw a5,  20(sp)
    lw a6,  24(sp)
    lw a7,  28(sp)
    lw t0,  32(sp)
    lw t1,  36(sp)
    lw t2,  40(sp)
    lw t3,  44(sp)
    lw t4,  48(sp)
    lw t5,  52(sp)
    lw t6,  56(sp)
    lw s0,  60(sp)
    lw s1,  64(sp)
    lw s2,  68(sp)
    lw s3,  72(sp)
    lw s4,  76(sp)
    lw s5,  80(sp)
    lw s6,  84(sp)
    lw s7,  88(sp)
    lw s8,  92(sp)
    lw s9,  96(sp)
    lw s10, 100(sp)
    lw s11, 104(sp)
    lw ra,  108(sp)
    lw gp,  112(sp)
    lw tp,  116(sp)

    /* Step6 */
    lw sp,  120(sp)

    /* Step7 */
    /* Student's code goes here (Multicore & Locks). */
    /* Release the kernel lock and make sure not to modify any registers, */
    /* so you may need to use sscratch just like how Step2 uses mscratch. */

    /* Student's code ends here. */

    /* Step8 */
    mret

.bss
    kernel_lock:     .word 0
