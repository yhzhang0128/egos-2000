#pragma once

struct earth {
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(const char *format, ...);    
};
