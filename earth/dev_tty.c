/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple tty device driver; getc and printf functions
 * have been implemented by the freedom-metal library for Arty
 */

#include <stdio.h>
#include <stdarg.h>
#include "bus_uart.h"

static int c, is_reading;

static void uart_set_clock(long baud_rate) {
    long cpu_clock_rate = 65000000;
    UART_REGW(METAL_SIFIVE_UART0_DIV) = cpu_clock_rate / baud_rate - 1;
    UART_REGW(METAL_SIFIVE_UART0_TXCTRL) |= 1;
    UART_REGW(METAL_SIFIVE_UART0_RXCTRL) |= 1;
}

static void uart_getc(int* c) {
    uint32_t ch = UART_REGW(METAL_SIFIVE_UART0_RXDATA);
    *c = (ch & UART_RXEMPTY)? -1 : (ch & 0x0ff);
}

int tty_init() {
    uart_set_clock(115200);

    /* Wait for the tty device to be ready */
    for (int i = 0; i < 2000000; i++);
    for (int c = 0; c != -1; uart_getc(&c));
    return 0;
}

int tty_intr() {
    if (is_reading) return 0;

    uart_getc(&c);
    return c == 3;
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int i = 0; i < len - 1; i++) {
        for (c = -1; c == -1; uart_getc(&c));
        buf[i] = (char)c;

        switch (c) {
        case 0x3:           /* Ctrl+C    */
            buf[0] = 0;
        case 0xd:           /* Enter     */
            buf[i] = 0;
            printf("\r\n");
            goto finish;
        case 0x7f:          /* Backspace */
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
    printf("%s[HIGHLIGHT] ", "\x1B[1;33m");
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int tty_success(const char *format, ...)
{
    printf("%s[SUCCESS] ", "\x1B[1;32m");
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int tty_fatal(const char *format, ...)
{
    printf("%s[FATAL] ", "\x1B[1;31m");
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
    while(1);
}

