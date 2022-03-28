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

#define ENTER  0x0d
#define CTRL_C 0x03

static struct metal_uart* uart;

static int is_intr, is_reading;
int tty_intr() {
    if (is_reading)
        return is_intr ? !(is_intr = 0) : 0;

    int c = -1;
    metal_uart_getc(uart, &c);
    if (c != -1 && (char)c == CTRL_C)
        return 1;
    else
        return 0;
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int c, i = 0; i < len - 1; i++) {
        for (c = -1; c == -1; metal_uart_getc(uart, &c));
        buf[i] = (char)c;

        switch (c) {
        case ENTER:
            printf("\r\n");
            buf[i] = 0;
            goto finish;
        case CTRL_C:
            printf("\r\n");
            buf[0] = 0;
            is_intr = 1;
            goto finish;
        }

        printf("%c", c);
        fflush(stdout);
    }

    buf[len - 1] = 0;

 finish:
    is_reading = 0;
    return 0;
}

int tty_write(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int r = vprintf(format, args);
    va_end(args);
    fflush(stdout);

    return r;
}

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
