/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: _enter of grass
 */
    .section .text
    .global grass_entry

grass_entry:
    li sp,0x80003f80
    call main
