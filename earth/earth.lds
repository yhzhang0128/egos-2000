OUTPUT_ARCH("riscv")

ENTRY(_enter)

MEMORY
{
    ram (arw!xi) : ORIGIN = 0x08000000, LENGTH = 0x2000
    rom (irx!wa) : ORIGIN = 0x20400000, LENGTH = 0x400000
}

PHDRS
{
    ram PT_LOAD;
    rom PT_LOAD;
    text PT_LOAD;
    ram_init PT_LOAD;
}

SECTIONS
{
    /* ROM Sections */
    .init : {
        *(.text.enter)
    } >rom :rom

    .text : {
        *(.text .text.*)
    } >rom :text

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
    PROVIDE( __heap_end = 0x08002800 );

    PROVIDE( bss_start = ADDR(.bss) );
    PROVIDE( bss_end = ADDR(.bss) + SIZEOF(.bss) );
    PROVIDE( data_rom = LOADADDR(.data) );
    PROVIDE( data_start = ADDR(.data) );
    PROVIDE( data_end = ADDR(.data) + SIZEOF(.data) );
}
