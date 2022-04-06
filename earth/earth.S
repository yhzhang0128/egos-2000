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

  la t1, metal_segment_bss_target_start
  la t2, metal_segment_bss_target_end
  bge t1, t2, 2f

  sw   x0, 0(t1)
  addi t1, t1, 4
  blt  t1, t2, 1b
2:

  /* Set TLS pointer */
  .weak __tls_base	
  la tp, __tls_base

  /* At this point we're in an environment that can execute C code.  The first
   * thing to do is to make the callback to the parent environment if it's been
   * requested to do so. */
  beqz a2, 1f
  jalr a2
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

  /* The delegation CSRs exist if user mode interrupts (N extension) or
   * supervisor mode (S extension) are supported */
  csrr a5, misa
  lui  a4, 0x42
  and  a4, a4, a5
  beqz a4, 1f
  csrc mideleg, a3
  csrc medeleg, a3
1:

  /* The satp CSR exists if supervisor mode (S extension) is supported */
  lui  a4, 0x40
  and  a4, a4, a5
  beqz a4, 1f
  csrc satp, a3
1:

  /* Check RISC-V isa and enable FS bits if Floating Point architecture. */
  li   a4, 0x10028
  and  a5, a5, a4
  beqz a5, 1f
  csrr a5, mstatus
  lui  a4, 0x2
  or   a5, a5, a4
  csrw mstatus, a5
  csrwi fcsr, 0
1:

  /* Check for vector extension support and enable it if found.
   * Omit if toolchain doesn't support the vector extension. */
#ifdef __riscv_v
  csrr a5, misa
  li a4, 0x200000
  and a5, a5, a4
  beqz a5, 1f
  csrr a5, mstatus
  ori a5, a5, 0x200
  csrw mstatus, a5
  vsetivli x0, 0, e8, m1, ta, ma
  csrwi vcsr, 0
1:
#endif

  /*call metal_secondary_init*/

  /* This is a C runtime, so main() is defined to have some arguments.  Since
   * there's nothing sane the METAL can pass we don't bother with that but
   * instead just setup as close to a NOP as we can. */
  li a0, 1     /* argc=1 */
  la a1, argv  /* argv = {"libgloss", NULL} */
  la a2, envp  /* envp = {NULL} */
  call secondary_main

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


/* RISC-V systems always use __libc_{init,fini}_array, but for compatibility we
 * define _{init,fini} to do nothing. */
.global _init
.type   _init, @function
.global _fini
.type   _fini, @function
_init:
_fini:
  ret
.size _init, .-_init
.size _fini, .-_fini

/* By default, secondary_main will cause secondary harts to spin forever.
 * Users can redefine secondary_main themselves to run code on secondary harts */
.weak   secondary_main
.type   secondary_main, @function

secondary_main:
  addi sp, sp, -16
#if __riscv_xlen == 32
  sw ra, 4(sp)
#else
  sd ra, 8(sp)
#endif
  csrr t0, mhartid
  la t1, __metal_boot_hart
  beq t0, t1, 2f
1:
  wfi
  j 1b
2:
  call main
#if __riscv_xlen == 32
  lw ra, 4(sp)
#else
  ld ra, 8(sp)
#endif
  addi sp, sp, 16
  ret

/* This shim allows main() to be passed a set of arguments that can satisfy the
 * requirements of the C API. */
.section .rodata.libgloss.start
.balign 8
argv:
.dc.a name
envp:
.dc.a 0
name:
.asciz "libgloss"

#ifdef __riscv_cmodel_compact
/* Compact stub.  */
  .section .text.__global_pointer__, "aMG",@progbits, 8, __global_pointer__, comdat
  .align 3
  .global __global_pointer__
  .type   __global_pointer__, object
__global_pointer__:
  .quad   __global_pointer$ -.
#endif
