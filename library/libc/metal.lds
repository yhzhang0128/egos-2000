OUTPUT_ARCH("riscv")

ENTRY(_enter)

MEMORY
{
    ram (arw!xi) : ORIGIN = 0x08000000, LENGTH = 0x3000
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
    __stack_size = 0x800;

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
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata2.*)
    } >ram AT>rom :ram_init

    .tdata : ALIGN(8) {
        PROVIDE( __tls_base = . );
    } >ram AT>rom :ram_init

    .tbss : ALIGN(8) {
    	*(.tcommon .tcommon.*)
	    PROVIDE( __tls_end = . );
    } >ram AT>ram :ram

    .tbss_space : ALIGN(8) {
    	. = . + __tbss_size;
    } >ram :ram

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >ram :ram 

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __end = . );
        PROVIDE( __heap_start = . );
        PROVIDE( metal_segment_heap_target_start = . );
        . +=  __heap_size;
        PROVIDE( metal_segment_heap_target_end = . );
        PROVIDE( _heap_end = . );
        PROVIDE( __heap_end = . );
    } >ram :ram

    .stack (NOLOAD) : ALIGN(16) {
        PROVIDE(metal_segment_stack_begin = .);
        . = __stack_size; /* Hart 0 */
        PROVIDE( _sp = . );
        PROVIDE(metal_segment_stack_end = .);
    } >ram :ram
    
    /* Symbols for the Metal library */
    PROVIDE(__metal_boot_hart = 0);
    PROVIDE(__metal_eccscrub_bit = 0);
    PROVIDE( metal_dtim_0_memory_start = 0x80000000 );
    PROVIDE( metal_dtim_0_memory_end = 0x80000000 + 0x4000 );
    PROVIDE( metal_itim_0_memory_start = 0x08000000 );
    PROVIDE( metal_itim_0_memory_end = 0x08000000 + 0x4000 );
    PROVIDE( metal_segment_itim_source_start = LOADADDR(.data) );
    PROVIDE( metal_segment_itim_target_start = ADDR(.data) );
    PROVIDE( metal_segment_itim_target_end = ADDR(.data) + SIZEOF(.data) );
    PROVIDE( metal_segment_lim_source_start = LOADADDR(.data) );
    PROVIDE( metal_segment_lim_target_start = ADDR(.data) );
    PROVIDE( metal_segment_lim_target_end = ADDR(.data) + SIZEOF(.data) );

    PROVIDE( __tdata_source = LOADADDR(.tdata) );
    PROVIDE( __tdata_size = SIZEOF(.tdata) );

    PROVIDE( metal_segment_data_source_start = LOADADDR(.data) );
    PROVIDE( metal_segment_data_target_start = ADDR(.data) );
    PROVIDE( metal_segment_data_target_end = ADDR(.tdata) + SIZEOF(.tdata) );
    PROVIDE( __tbss_size = SIZEOF(.tbss) );
    PROVIDE( __tls_size = __tls_end - __tls_base );
    PROVIDE( metal_segment_bss_source_start = LOADADDR(.tbss) );
    PROVIDE( metal_segment_bss_target_start = ADDR(.tbss) );
    PROVIDE( metal_segment_bss_target_end = ADDR(.bss) + SIZEOF(.bss) );

    /DISCARD/ : {
	*(.eh_frame .eh_frame.*)
    }
}
