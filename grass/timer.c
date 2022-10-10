/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: timer reset and initialization
 * mtime is at 0x200bff8 and mtimecmp is at 0x2004000 in the memory map
 * see section 3.1.15 of the RISC-V manual, volume2, v1.10
 * and section 9.1, 9.3 of the Sifive FE310 manual, v19p04
 */

#define QUANTUM  5000

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

void timer_init()  { mtimecmp_set(0); }
void timer_reset() { mtimecmp_set(mtime_get() + QUANTUM); }
