#pragma once

/* SiFive FE310 has a 65MHz clock */
#define CPU_CLOCK_RATE 65000000

struct earth {
    /* CPU interface */
    int (*intr_enable)();
    int (*intr_register)(void (*handler)(int));
    int (*excp_register)(void (*handler)(int));

    int (*mmu_alloc)(int* frame_no, int* cached_addr);
    int (*mmu_free)(int pid);
    int (*mmu_map)(int pid, int page_no, int frame_no);
    int (*mmu_switch)(int pid);

    /* Disk and tty device driver interface */
    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*tty_intr)();
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);

    int (*tty_info)(const char *format, ...);
    int (*tty_fatal)(const char *format, ...);
    int (*tty_success)(const char *format, ...);
    int (*tty_highlight)(const char *format, ...);
};

struct grass {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);

    int work_dir_ino;
    char work_dir_name[32];
};

extern struct earth *earth;

#define INFO      earth->tty_info
#define FATAL     earth->tty_fatal
#define SUCCESS   earth->tty_success
#define HIGHLIGHT earth->tty_highlight
#define printf    earth->tty_write
