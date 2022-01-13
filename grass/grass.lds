/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory layout of the grass kernel
 */

OUTPUT_ARCH("riscv")

ENTRY(_enter)

MEMORY
{
    ram (arw!xi) : ORIGIN = 0x8004000, LENGTH = 0x4000
}

PHDRS
{
    ram PT_LOAD;
}

SECTIONS
{
    .text : ALIGN(8) {
        *(.text .text.*)
    } >ram :ram

    .rodata : ALIGN(8) {
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata.cst16)
        *(.srodata.cst8)
        *(.srodata.cst4)
        *(.srodata.cst2)
        *(.srodata .srodata.*)
    } >ram :ram

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >ram : ram

    .bss (NOLOAD): ALIGN(8) {
        *(.bss .bss.*)
        *(COMMON)
    } >ram :ram

    .heap (NOLOAD) : ALIGN(8) {
        . = 0x1000;
    } >ram :ram


    .stack (NOLOAD) : ALIGN(16) {
        . += 0x2000;
    } >ram :ram
}
