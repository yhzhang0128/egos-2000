.section .text.enter
.global _enter
_enter:
    /* SiFive specific initialization */
    csrwi 0x7C1, 0
    /* Setup stack pointer and enter libc */
    li sp, 0x08003000
    li a0, 0
    li a1, 0
    li a2, 0
    call _start

/* Temporary functions for linking with metal-gloss
 */
metal_init_run:
    ret
metal_fini_run:
    ret

