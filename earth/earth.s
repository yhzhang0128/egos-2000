/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader & trap entry
 */
    .section .image.placeholder
    .section .text.enter
    .global earth_entry, trap_from_M_mode, trap_from_S_mode

earth_entry:
    li t0, 0x8         /* The first instruction during boot up */
    csrc mstatus, t0   /* Disable interrupt */
    li sp, 0x80003f80
    call main

trap_from_S_mode:
    /* Set mstatus.MPRV to enable page table translation in M mode */
    /* If mstatus.MPP is U mode, set to S mode for kernel privilege */
    csrw mscratch, t0
    li t0, 0x20800
    csrs mstatus, t0
    csrr t0, mscratch

trap_from_M_mode:
    /* Step1: switch to the kernel stack
       Step2: save all registers on the kernel stack
       Step3: call trap_entry()
       Step4: restore all registers
       Step5: switch back to the user stack
       Step6: invoke the mret instruction*/
    csrw mscratch, sp  /* Step1 */
    lui sp, 0x80004
    addi sp, sp, -128  /* Kernel stack is 0x80003f80 */
    addi sp, sp, -116  /* Step2 */
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
    csrr t0, mscratch
    sw t0, 112(sp)      /* Save user sp on kernel stack */

    call trap_entry     /* Step3 */

    lw ra, 0(sp)        /* Step4 */
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
    lw sp, 112(sp)      /* Step5 */
   mret                 /* Step6 */
