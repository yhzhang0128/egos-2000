/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: timer reset and initialization
 * mtime is at 0x200bff8 and mtimecmp is at 0x2004000 in the memory map
 * see section 3.1.15 of references/riscv-privileged-v1.10.pdf
 * and section 9.1, 9.3 of references/sifive-fe310-v19p04.pdf
 */

#include "egos.h"

static long long mtime_get() {
    int low, high;
    do {
        high = *(int*)(0x200bff8 + 4);
        low  = *(int*)(0x200bff8);
    }  while ( *(int*)(0x200bff8 + 4) != high );

    return (((long long)high) << 32) | low;
}

static void mtimecmp_set(long long time) {
    *(int*)(0x2004000 + 4) = 0xFFFFFFFF;
    *(int*)(0x2004000 + 0) = (int)time;
    *(int*)(0x2004000 + 4) = (int)(time >> 32);
}

int timer_reset() {
    mtimecmp_set(mtime_get() + 50000);
    return 0;
}

void timer_init()  {
    earth->timer_reset = timer_reset;
    mtimecmp_set(0x0FFFFFFFFFFFFFFFUL);
}
