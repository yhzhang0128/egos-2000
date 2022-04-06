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
    __heap_size = 0x800;

    /* ROM Sections */
    .init : {
        KEEP (*(.text.enter))
    } >rom :rom

    .rodata : {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >rom :rom

    .text : {
        *(.text .text.*)
    } >rom :text

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
        PROVIDE( metal_segment_heap_target_start = . );
        . +=  __heap_size;
        PROVIDE( metal_segment_heap_target_end = . );
        PROVIDE( __heap_end = . );
    } >ram :ram
    
    PROVIDE( bss_start = ADDR(.bss) );
    PROVIDE( bss_end = ADDR(.bss) + SIZEOF(.bss) );
    PROVIDE( data_rom_start = LOADADDR(.data) );
    PROVIDE( data_ram_start = ADDR(.data) );
    PROVIDE( data_ram_end = ADDR(.data) + SIZEOF(.data) );
}
