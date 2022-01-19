/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: provide the _sbrk function to glibc malloc()
 */


#include <stddef.h>

/* heap region is defined in the memory layout scripts */
extern char __heap_start;
extern char __heap_end;

static char* brk = &__heap_start;

char* _sbrk(ptrdiff_t incr) {
    char* old = brk;

    if (brk + incr >= &__heap_end)
        return (void*) -1;

    brk += incr;
    return old;
}
