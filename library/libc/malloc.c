/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: system support to C library function malloc()
 */

#include "egos.h"

extern char __heap_start, __heap_end;
char* brk = &__heap_start;

char *_sbrk(int size) {
    if ((brk + size) > (&__heap_end)) {
        earth->tty_write("_sbrk: heap is full\r\n", 21);
        while(1);
    }

    char *old_brk = brk;
    brk += size;
    for (int i = 0; i < size; i++) old_brk[i] = 0;

    return old_brk;
}
