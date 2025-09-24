OUTPUT_ARCH("riscv")

ENTRY(boot_loader)

MEMORY
{
    code (rx) : ORIGIN = 0x80000000, LENGTH = 0x8000
    data (rw) : ORIGIN = 0x80008000, LENGTH = 0xF8000
}

PHDRS
{
    code PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    .init : ALIGN(8) {
        *(.text.enter)
    } >code :code

    .text : ALIGN(8) {
        *(.text .text.*)
    } >code :code

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >data :data

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >data :data

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >data :data

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __heap_start = . );
    } >data :data

    PROVIDE( __heap_end = 0x80100000 );
}
