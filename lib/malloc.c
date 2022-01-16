/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: provide the _sbrk function to glibc malloc()
 */


#include <stddef.h>

/* heap region is defined in the memory layout scripts */
extern char HEAP_START;
extern char HEAP_END;

static char* brk = &HEAP_START;

char* _sbrk(ptrdiff_t incr) {
    char* old = brk;

    if (brk + incr >= &HEAP_END)
        return (void*) -1;

    brk += incr;
    return old;
}
