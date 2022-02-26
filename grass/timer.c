/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: read mtime register and write mtimecmp register;
 * see section 3.1.15 of the RISC-V manual, volume2, v1.10
 */

#include "egos.h"
#include "grass.h"

#define ACCESS(x) (*(__typeof__(*(unsigned int*)x) volatile *)((unsigned int*)(x)))

static long long mtime_get() {
    unsigned int time_lo, time_hi;
    /* Guard against rollover when reading */
    do {
        time_hi = ACCESS(RISCV_CLINT0_MTIME_BASE + 4);
        time_lo = ACCESS(RISCV_CLINT0_MTIME_BASE);
    } while (ACCESS(RISCV_CLINT0_MTIME_BASE + 4) != time_hi);

    return (((unsigned long long)time_hi) << 32) | time_lo;
}

static void mtimecmp_set(long long time) {
    ACCESS(RISCV_CLINT0_MTIMECMP_BASE + 4) = 0xFFFFFFFF;
    ACCESS(RISCV_CLINT0_MTIMECMP_BASE) = (unsigned int)time;
    ACCESS(RISCV_CLINT0_MTIMECMP_BASE + 4) = (unsigned int)(time >> 32);
}

void timer_init() {
    mtimecmp_set(0);
}

long long timer_reset() {
    long long mtime = mtime_get();
    mtimecmp_set(mtime + QUANTUM_NCYCLES);
    return mtime;
}
