/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: entry point of the bootloader
 */
    .section .text.enter
    .global _start

_start:
// 設定 stack pointer 的位置, 可以自己調整看看
    li sp, 0x80400000
    call main
_end:
    call _end
