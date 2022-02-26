#include "egos.h"
#include "grass.h"

#define CLINT_BASE 0x2000000
#define METAL_RISCV_CLINT0_MTIME 49144UL
#define METAL_RISCV_CLINT0_MTIMECMP_BASE 16384UL
#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))

static long long mtime_get() {
    unsigned int time_lo, time_hi;
    /* Guard against rollover when reading */
    do {
        time_hi = __METAL_ACCESS_ONCE(
            (unsigned int*)(CLINT_BASE + METAL_RISCV_CLINT0_MTIME + 4));
        time_lo = __METAL_ACCESS_ONCE(
            (unsigned int*)(CLINT_BASE + METAL_RISCV_CLINT0_MTIME));
    } while (__METAL_ACCESS_ONCE((unsigned int*)(CLINT_BASE +
                                                    METAL_RISCV_CLINT0_MTIME +
                                                    4)) != time_hi);

    return (((unsigned long long)time_hi) << 32) | time_lo;
}

static void mtimecmp_set(long long time) {
    __METAL_ACCESS_ONCE((unsigned int*)(CLINT_BASE +
                                        METAL_RISCV_CLINT0_MTIMECMP_BASE +
                                        4)) = 0xFFFFFFFF;
    __METAL_ACCESS_ONCE((unsigned int*)(CLINT_BASE +
                                        METAL_RISCV_CLINT0_MTIMECMP_BASE)) = (unsigned int)time;
    __METAL_ACCESS_ONCE((unsigned int*)(CLINT_BASE +
                                           METAL_RISCV_CLINT0_MTIMECMP_BASE +
                                           4)) = (unsigned int)(time >> 32);
}

long long timer_reset() {
    long long mtime = mtime_get();
    mtimecmp_set(mtime + QUANTUM_NCYCLES);
    return mtime;
}
