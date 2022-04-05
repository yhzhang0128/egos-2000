.section .text.enter
.global _enter
_enter:
    /* Initialize global pointer */
    la gp, __global_pointer$
    /* SiFive specific initialization */
    csrwi 0x7C1, 0
    /* Setup stack pointer and enter libc */
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

/* Temporary functions for linking with metal-gloss
 */
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

