#pragma once

struct dev_log {
    int (*log_info)(const char *format, ...);
    int (*log_highlight)(const char *format, ...);
    int (*log_success)(const char *format, ...);
    int (*log_fatal)(const char *format, ...);
};

struct earth {
    /* TTY and disk device driver interface */
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    /* CPU interface */
    int (*intr_enable)();
    int (*intr_disable)();
    int (*intr_register)(void (*handler)(int));
    int (*excp_register)(void (*handler)(int));

    int (*mmu_free)(int pid);
    int (*mmu_alloc)(int* frame_no, int* addr);
    int (*mmu_switch)(int pid);
    int (*mmu_map)(int pid, int page_no, int frame_no, int flag);

    /* helper functions for logging */
    struct dev_log log;
};

struct grass {
    int work_dir_ino;
    char work_dir[32 * 16];
    char work_dir_name[32];
};
