#pragma once

struct earth {
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);    

    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*intr_enable)();
    int (*intr_disable)();
};
