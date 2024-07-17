/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a simple tty device driver
 * uart_getc() and uart_putc() are implemented in bus_uart.c
 * printf-related functions are linked from the compiler's C library
 */

#include "egos.h"
#include <stdio.h>

void uart_getc(int* c);
void uart_putc(int c);

int tty_write(char* buf, uint len) {
    for (uint i = 0; i < len; i++) uart_putc(buf[i]);
    return len;
}

int tty_read(char* buf, uint len) {
    for (int i = 0, c; i < len - 1; i++) {
        uart_getc(&c);
        buf[i] = (char)c;

        switch (c) {
        case 0x03:  /* Ctrl+C    */
            buf[0] = 0;
        case 0x0d:  /* Enter     */
            buf[i] = 0;
            tty_write("\r\n", 2);
            return c == 0x03? 0 : i;
        case 0x7f:  /* Backspace */
            c = 0;
            if (i) tty_write("\b \b", 3);
            i = i ? i - 2 : i - 1;
        }
        if (c) tty_write((void*)&c, 1);
    }

    buf[len - 1] = 0;
    return len - 1;
}

void tty_init() {
    earth->tty_read = tty_read;
    earth->tty_write = tty_write;
    earth->tty_vsprintf = vsprintf;
}
