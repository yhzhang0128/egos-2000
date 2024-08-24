/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: system support to C library function malloc()
 */

#include "egos.h"

/* Heap start/end are defined in library/linker/*.lds */
extern char __heap_start, __heap_end;
static char* brk = &__heap_start;

/* malloc() and free() are linked from the compiler's C library;
 * malloc() and free() manage the memory region [&__heap_start, brk).
 * If malloc() finds this region too small, it will increase brk by call _sbrk().
 */

char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        printf("_sbrk: heap grows too large\r\n");
        *(int*)(0) = 1; /* Trigger a memory exception */
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}
