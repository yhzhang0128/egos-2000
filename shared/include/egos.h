#pragma once

#include "log.h"

#define TIMER_INTR_ID   7
#define QUANTUM_NCYCLES 2000

#define F_INUSE         0x1
#define F_READ          0x2
#define F_WRITE         0x4
#define F_EXEC          0x8
#define F_ALL           0xf

#define EARTH_ADDR      (0x8004000 - 0x40)

typedef void (*handler_t)(int, void*);

struct earth {
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*intr_enable)();
    int (*intr_disable)();
    int (*intr_register)(int id, handler_t handler);

    int (*mmu_alloc)(int* frame_no, int* addr);
    int (*mmu_map)(int pid, int page_no, int frame_no, int flag);
    int (*mmu_switch)(int pid);

    struct dev_log log;
};
