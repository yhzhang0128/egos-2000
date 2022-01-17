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
    __heap_size = 0x400;
    __stack_size = 0x2000;

    .text : ALIGN(8) {
        *(.text .text.*)
    } >ram :ram

    .rodata : ALIGN(8) {
        *(.rdata)
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
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >ram :ram

    .heap (NOLOAD) : ALIGN(8) {
          PROVIDE( HEAP_START = . );
          . = __heap_size;
          PROVIDE( HEAP_END = . );
    } >ram :ram


    .stack (NOLOAD) : ALIGN(16) {
        . += __stack_size;
    } >ram :ram
}
