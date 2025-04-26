/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: system support for C library function malloc()
 */

#include "egos.h"

/* Heap start and end are defined in library/elf/{egos/app}.lds. */
extern char __heap_start, __heap_end;
static char* brk = &__heap_start;

/* malloc() and free() are linked from the compiler's C library;
 * malloc() and free() manage the memory region [&__heap_start, brk).
 * If malloc() finds it too small, malloc() will call _sbrk() to increase brk.
 */

char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        printf("_sbrk: heap grows too large\r\n");
        *(int*)(0) = 1; /* Trigger a memory exception. */
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}
