OUTPUT_ARCH("riscv")

ENTRY(_enter)

MEMORY
{
    ram (arw!xi) : ORIGIN = 0x08000000, LENGTH = 0x2000
    rom (irx!wa) : ORIGIN = 0x20400000, LENGTH = 0x400000
    disk(irx!wa) : ORIGIN = 0x20800000, LENGTH = 0x400000
}

PHDRS
{
    rom PT_LOAD;
    ram_init PT_LOAD;
    ram PT_LOAD;
    disk PT_LOAD;
}

SECTIONS
{
    /* ROM Sections */
    .init : {
        *(.text.enter)
    } >rom :rom

    .text : {
        *(.text .text.*)
    } >rom :rom

    .rodata : {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >rom :rom

    /* RAM Sections */
    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >ram AT>rom :ram_init

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >ram :ram 

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __heap_start = . );
    } >ram :ram

    .image : {
        *(.image.placeholder);
        . += 0x400000;
    } >disk :disk

    PROVIDE( bss_start = ADDR(.bss) );
    PROVIDE( bss_end = ADDR(.bss) + SIZEOF(.bss) );
    PROVIDE( data_rom = LOADADDR(.data) );
    PROVIDE( data_start = ADDR(.data) );
    PROVIDE( data_end = ADDR(.data) + SIZEOF(.data) );
    PROVIDE( __heap_end = ADDR(.data) + 0x2800 );
}
