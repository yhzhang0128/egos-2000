#pragma once

#include "log.h"

#define INTR_ID_TMR     7
#define INTR_ID_SOFT    3
#define QUANTUM_NCYCLES 5000

#define F_INUSE         0x1
#define F_READ          0x2
#define F_WRITE         0x4
#define F_EXEC          0x8
#define F_ALL           0xf

#define EARTH_ADDR      (0x08008000 - 0x80)

typedef void (*handler_t)(int);

struct earth {
    /* TTY and disk device driver interface */
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);
    int (*disk_busy)();

    /* CPU interface */
    int (*intr_enable)();
    int (*intr_disable)();
    int (*intr_register)(handler_t handler);
    int (*excp_register)(handler_t handler);

    int (*mmu_alloc)(int* frame_no, int* addr);
    int (*mmu_map)(int pid, int page_no, int frame_no, int flag);
    int (*mmu_switch)(int pid);

    /* helper functions for logging */
    struct dev_log log;
};


enum {
    GPID_UNUSED,
    GPID_PROCESS,
    GPID_FILE,
    GPID_DIR,
    GPID_SHELL,
    GPID_USER_START
}; 

void ctx_switch(void** old_sp, void* new_sp);
void ctx_start(void** old_sp, void* new_sp);
void ctx_entry(void);
