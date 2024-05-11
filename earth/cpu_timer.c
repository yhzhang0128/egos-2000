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

static ulonglong mtime_get() {
    uint low, high;
    do {
        high = REGW(0x200BFF8, 4);
        low  = REGW(0x200BFF8, 0);
    }  while ( REGW(0x200BFF8, 4) != high );

    return (((ulonglong)high) << 32) | low;
}

static int mtimecmp_set(ulonglong time) {
    uint mhartid;
    asm("csrr %0, mhartid" : "=r"(mhartid));

    REGW(0x2004000, mhartid * 8 + 4) = 0xFFFFFFFF;
    REGW(0x2004000, mhartid * 8 + 0) = (uint)time;
    REGW(0x2004000, mhartid * 8 + 4) = (uint)(time >> 32);

    return 0;
}

static uint QUANTUM;
int timer_reset() { return mtimecmp_set(mtime_get() + QUANTUM); }

void timer_init()  {
    earth->timer_reset = timer_reset;
    QUANTUM = (earth->platform == ARTY)? 5000 : 500000;
    mtimecmp_set(0x0FFFFFFFFFFFFFFFUL);
}
