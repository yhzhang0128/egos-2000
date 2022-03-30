/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: naive malloc and free; a better one is left to students
 */


#include "egos.h"
#include "memory.h"

/* heap region is defined in the memory layout scripts */
extern char __heap_start;
extern char __heap_end;
static char* brk = &__heap_start;

void* my_alloc(unsigned int size) {
    char* old = brk;

    if (brk + size >= &__heap_end) FATAL("Heap is full");

    brk += size;
    return old;    
}

void my_free(void* ptr) {}
