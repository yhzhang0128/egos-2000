OUTPUT_ARCH("riscv")

ENTRY(boot_loader)

MEMORY
{
    ram (arw!xi) : ORIGIN = 0x80000000, LENGTH = 0x200000
}

PHDRS
{
    ram PT_LOAD;
}

SECTIONS
{
    .init : ALIGN(8) {
        *(.text.enter)
    } >ram :ram

    .text : ALIGN(8) {
        *(.text .text.*)
    } >ram :ram

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >ram :ram

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >ram :ram

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >ram :ram 

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __heap_start = . );
    } >ram :ram

    PROVIDE( __heap_end = 0x80200000 );
}
