/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: system support to C library function malloc()
 */

#include "egos.h"

extern char __heap_start, __heap_end;
static char* brk = &__heap_start;

/* malloc() and free() are linked from the compiler's C library;
 * malloc() and free() manage the memory region [&__heap_start, brk).
 * If malloc() finds this region to be too small, it will call _sbrk().
 */

char *_sbrk(int size) {
    char* heap_end = (earth->platform == QEMU)? (char*)0xa000000: (&__heap_end);
    if (brk + size > heap_end) {
        earth->tty_write("_sbrk: heap grows too large\r\n", 29);
        *(int*)(0xFFFFFFF0) = 1; /* Trigger a memory exception */
    }

    char *old_brk = brk;
    brk += size;
    return old_brk;
}
