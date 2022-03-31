/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: read mtime register and write mtimecmp register;
 * see section 3.1.15 of the RISC-V manual, volume2, v1.10
 * and section 9.1, 9.3 of the Sifive FE310 manual, v19p04
 */

#include "egos.h"
#include "grass.h"

#define QUANTUM_NCYCLES            5000
#define CLINT0_MTIME_BASE          0x200bff8
#define CLINT0_MTIMECMP_BASE       0x2004000
#define ACCESS(x) (*(__typeof__(*(unsigned int*)x) volatile *)((unsigned int*)(x)))

static long long mtime_get() {
    unsigned int time_lo, time_hi;
    /* Guard against rollover when reading mtime */
    do {
        time_hi = ACCESS(CLINT0_MTIME_BASE + 4);
        time_lo = ACCESS(CLINT0_MTIME_BASE);
    } while (ACCESS(CLINT0_MTIME_BASE + 4) != time_hi);

    return (((unsigned long long)time_hi) << 32) | time_lo;
}

static void mtimecmp_set(long long time) {
    ACCESS(CLINT0_MTIMECMP_BASE + 4) = 0xFFFFFFFF;
    ACCESS(CLINT0_MTIMECMP_BASE) = (unsigned int)time;
    ACCESS(CLINT0_MTIMECMP_BASE + 4) = (unsigned int)(time >> 32);
}

void timer_init() {
    mtimecmp_set(0);
}

void timer_reset() {
    mtimecmp_set(mtime_get() + QUANTUM_NCYCLES);
}
