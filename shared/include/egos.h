#pragma once

/* Interface between earth and grass */

#define TIMER_INTR_ID   7
#define QUANTUM_NCYCLES 2000
#define EARTH_ADDR      (0x8004000 - 0x40)

typedef void (*handler_t)(int, void*);

struct dev_log {
    int (*log_info)(const char *format, ...);
    int (*log_highlight)(const char *format, ...);
    int (*log_success)(const char *format, ...);
    int (*log_error)(const char *format, ...);
    int (*log_fatal)(const char *format, ...);
};

struct earth {
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*intr_enable)();
    int (*intr_disable)();
    int (*intr_register)(int id, handler_t handler);

    struct dev_log log;
};

/* Interface between grass and apps */
