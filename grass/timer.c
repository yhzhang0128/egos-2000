/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Initialize and reset timer;
 * see section 3.1.15 of the RISC-V manual, volume2, v1.10
 * and section 9.1, 9.3 of the Sifive FE310 manual, v19p04
 */

#include "egos.h"

#define QUANTUM  5000
#define CLINT0_MTIME     0x200bff8
#define CLINT0_MTIMECMP  0x2004000

static long long mtime_get() {
    int low, high;
    do {
        high = *(int*)(CLINT0_MTIME + 4);
        low  = *(int*)(CLINT0_MTIME);
    }  while ( *(int*)(CLINT0_MTIME + 4) != high );

    return (((long long)high) << 32) | low;
}

static void mtimecmp_set(long long time) {
    *(int*)(CLINT0_MTIMECMP + 4) = 0xFFFFFFFF;
    *(int*)(CLINT0_MTIMECMP + 0) = (int)time;
    *(int*)(CLINT0_MTIMECMP + 4) = (int)(time >> 32);
}

void timer_init()  { mtimecmp_set(0); }
void timer_reset() { mtimecmp_set(mtime_get() + QUANTUM); }
