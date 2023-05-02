/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: _enter of grass
 */
    .section .text
    .global _enter
_enter:
    li sp,0x80003f80
    call main
