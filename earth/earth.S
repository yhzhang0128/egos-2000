/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader
 * i.e., the first instructions executed by the processor when boot up
 */
    .section .text.enter
    .global _enter
_enter:
    /* Setup the stack pointer */
    li sp, 0x08003000

    /* First argument of _start is the core ID;
     * SiFive FE310 has only 1 core so the ID is 0 */
    li a0, 0

    /* Second argument of _start is ignored;
       Third argument needs to be initialized as 0 */
    li a2, 0

    /* Enter the C library (in library/libc/libmetal-gloss.a).
     * The C library will do some initialization and then call main() */
    call _start

_start:
  /* Stack pointer is expected to be initialized before _start */

  la t0, metal_segment_data_source_start
  la t1, metal_segment_data_target_start
  la t2, metal_segment_data_target_end

  beq t0, t1, 2f
  bge t1, t2, 2f

1:
  lw   a0, 0(t0)
  addi t0, t0, 4
  sw   a0, 0(t1)
  addi t1, t1, 4
  blt  t1, t2, 1b
2:

  /* Copy the ITIM section */
1:
2:
2:

  /* Copy the LIM section */
1:
2:

  /* Zero the BSS segment. */
  la t1, metal_segment_bss_target_start
  la t2, metal_segment_bss_target_end
  bge t1, t2, 2f

1:
  sw   x0, 0(t1)
  addi t1, t1, 4
  blt  t1, t2, 1b
2:

1:

  /* The RISC-V port only uses new-style constructors and destructors. */

  /* Register metal_fini_run as a destructor and call metal_init_run to
   * run and setup Metal constructors */

_skip_init:

  /* Synchronize harts so that secondary harts wait until hart 0 finishes
     initializing */
  /*call __metal_synchronize_harts*/

  /* Disable and clear all interrupt sources */
  li   a3, -1
  csrc mie, a3
  csrc mip, a3

1:

1:
1:

  /* Check for vector extension support and enable it if found.
   * Omit if toolchain doesn't support the vector extension. */
#ifdef __riscv_v
1:
#endif

  /*call metal_secondary_init*/

  /* This is a C runtime, so main() is defined to have some arguments.  Since
   * there's nothing sane the METAL can pass we don't bother with that but
   * instead just setup as close to a NOP as we can. */
  li a0, 0     /* argc=0 */
  call main

  /* Call exit to handle libc's cleanup routines.  Under normal contains this
   * shouldn't even get called, but I'm still not using a tail call here
   * because returning to the METAL is the right thing to do in pathological
   * situations. */
  call exit

  /* And here's where we return.  Again, it's a bit odd but the METAL defines
   * this as a bad idea (ie, as opposed to leaving it undefined) and at this
   * point it's really the only thing left to do. */
  mv ra, s0
  ret
