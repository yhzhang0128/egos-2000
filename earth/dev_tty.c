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
#define DELETE 0x7f

static int is_reading;
static struct metal_uart* uart;

int tty_intr() {
    if (is_reading)
        return 0;

    int c = -1;
    metal_uart_getc(uart, &c);
    return c == CTRL_C;
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int c, i = 0; i < len - 1; i++) {
        for (c = -1; c == -1; metal_uart_getc(uart, &c));
        buf[i] = (char)c;

        switch (c) {
        case CTRL_C:
            buf[0] = 0;
        case ENTER:
            buf[i] = 0;
            printf("\r\n");
            goto finish;
        case DELETE:
            c = 0;
            if (i) printf("\b \b");
            i = i ? i - 2 : i - 1;
        }

        if (c) printf("%c", c);
        fflush(stdout);
    }

 finish:
    buf[len - 1] = 0;
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
