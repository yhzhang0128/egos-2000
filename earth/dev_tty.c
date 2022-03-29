/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple tty device driver; getc and printf functions
 * have been implemented by the freedom-metal library for Arty
 */

#include "earth.h"
#include "bus_uart.h"

static int c, is_reading;
static struct metal_uart* uart;

int tty_init() {
    uart = metal_uart_get_device(0);
    metal_uart_init(uart, 115200);
    for (int i = 0; i < 2000000; i++);
    
    if (!uart)
        FATAL("Unable to get uart handle");

    /* wait for the tty device to be ready */
    for (int c = 0; c != -1; metal_uart_getc(uart, &c));
    return 0;
}

int tty_intr() {
    if (is_reading) return 0;

    metal_uart_getc(uart, &c);
    return c == 3;          // Ctrl + C
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int i = 0; i < len - 1; i++) {
        for (c = -1; c == -1; metal_uart_getc(uart, &c));
        buf[i] = (char)c;

        switch (c) {
        case 0x3:           // Ctrl + C
            buf[0] = 0;
        case 0xd:           // Enter
            buf[i] = 0;
            printf("\r\n");
            goto finish;
        case 0x7f:          // Backspace / Delete
            c = 0;
            if (i) printf("\b \b");
            i = i ? i - 2 : i - 1;
        }

        if (c) printf("%c", c);
        fflush(stdout);
    }

 finish:
    buf[len - 1] = is_reading = 0;    
    return 0;
}

#define VPRINTF   va_list args; \
                  va_start(args, format); \
                  vprintf(format, args); \
                  va_end(args); \

int tty_write(const char *format, ...)
{
    VPRINTF
    fflush(stdout);
}

int tty_info(const char *format, ...)
{
    printf("[INFO] ");
    VPRINTF
    printf("\r\n");
}

int tty_highlight(const char *format, ...)
{
    printf("%s[HIGHLIGHT] ", "\x1B[1;33m"); // yellow
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int tty_success(const char *format, ...)
{
    printf("%s[SUCCESS] ", "\x1B[1;32m");   // green
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int tty_fatal(const char *format, ...)
{
    printf("%s[FATAL] ", "\x1B[1;31m");     // red
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
    while(1);
}

