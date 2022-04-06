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

char *_sbrk(int size) {
    if ((brk + size) > (&__heap_end)) {
        earth->tty_write("\r\n[FATAL] _sbrk: heap is full\r\n", 31);
        *(int*)(0x1000) = 1; /* Trigger a memory exception */
    }

    char *old_brk = brk;
    brk += size;
    return old_brk;
}
