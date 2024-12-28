/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the kernel
 * When getting an interrupt or exception, the CPU sets its program counter to this entry point
 */
    .section .text
    .global trap_entry, trap_entry_using_page_table_translation

trap_entry:
    /* Step1: switch to the kernel stack
     * Step2: acquire kernel_lock (only for multicore in P8)
     * Step3: save all the registers on the kernel stack
     * Step4: call kernel_entry()
     * Step5: restore all the registers
     * Step6: switch back to the process stack
     * Step7: release kernel_lock (only for multicore in P8)
     * Step8: invoke mret and return to the process context */

    /* Step1 */
    csrw mscratch, sp
    li sp, 0x80400000

    /* Step2 */
    /* Ignore this step before you start to do P8 */
    /* Student's code goes here (multi-core and atomic instruction)  */
    /* Acquire kernel_lock and make sure not to modify any registers, */
    /* so you may need to use sscratch just like Step1 used mscratch. */

    /* Student's code ends here. */

    /* Step3 */
    addi sp, sp, -116  /* sp == SAVED_REGISTER_ADDR */
    sw ra, 0(sp)
    sw t0, 4(sp)
    sw t1, 8(sp)
    sw t2, 12(sp)
    sw t3, 16(sp)
    sw t4, 20(sp)
    sw t5, 24(sp)
    sw t6, 28(sp)
    sw a0, 32(sp)
    sw a1, 36(sp)
    sw a2, 40(sp)
    sw a3, 44(sp)
    sw a4, 48(sp)
    sw a5, 52(sp)
    sw a6, 56(sp)
    sw a7, 60(sp)
    sw s0, 64(sp)
    sw s1, 68(sp)
    sw s2, 72(sp)
    sw s3, 76(sp)
    sw s4, 80(sp)
    sw s5, 84(sp)
    sw s6, 88(sp)
    sw s7, 92(sp)
    sw s8, 96(sp)
    sw s9, 100(sp)
    sw s10, 104(sp)
    sw s11, 108(sp)
    csrr t0, mscratch  /* Step1 has written sp to mscratch */
    sw t0, 112(sp)     /* t0 holds the value of sp before trap_entry */

    /* Step4 */
    csrr a0, mcause
    call kernel_entry

    /* Step5 */
    lw ra, 0(sp)
    lw t0, 4(sp)
    lw t1, 8(sp)
    lw t2, 12(sp)
    lw t3, 16(sp)
    lw t4, 20(sp)
    lw t5, 24(sp)
    lw t6, 28(sp)
    lw a0, 32(sp)
    lw a1, 36(sp)
    lw a2, 40(sp)
    lw a3, 44(sp)
    lw a4, 48(sp)
    lw a5, 52(sp)
    lw a6, 56(sp)
    lw a7, 60(sp)
    lw s0, 64(sp)
    lw s1, 68(sp)
    lw s2, 72(sp)
    lw s3, 76(sp)
    lw s4, 80(sp)
    lw s5, 84(sp)
    lw s6, 88(sp)
    lw s7, 92(sp)
    lw s8, 96(sp)
    lw s9, 100(sp)
    lw s10, 104(sp)
    lw s11, 108(sp)

    /* Step6 */
    lw sp, 112(sp)

    /* Step7 */
    /* Ignore this step before you start to do P8 */
    /* Student's code goes here (multi-core and atomic instruction) */
    /* Release kernel_lock and make sure not to modify any registers, */
    /* so you may need to use sscratch just like Step1 used mscratch. */

    /* Student's code ends here. */

    /* Step8 */
    mret

/* Ignore the code below before you start to do P4 */
trap_entry_using_page_table_translation:
    /* Set mstatus.MPRV to enable page table translation in M mode  */
    /* If mstatus.MPP is U mode, set to S mode for kernel privilege */
    csrw mscratch, t0
    li t0, 0x20800
    csrs mstatus, t0
    csrr t0, mscratch
    j trap_entry
