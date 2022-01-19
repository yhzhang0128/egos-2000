#pragma once

#define TIMER_INTR_ID 7
#define QUANTUM_NCYCLES 2000
typedef void (*handler_t)(int, void*);

struct earth {
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);    

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*intr_enable)();
    int (*intr_disable)();
    int (*intr_register)(int id, handler_t handler);
};
