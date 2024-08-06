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

/* UART functions defined in earth/bus_uart.c */
void uart_getc(int* c);
void uart_putc(int c);

int tty_write(char* buf, uint len) {
    for (uint i = 0; i < len; i++) uart_putc(buf[i]);
    return len;
}

int tty_read(char* c) {
    uart_getc((int*)c);
}

void tty_init() {
    earth->tty_read = tty_read;
    earth->tty_write = tty_write;
    earth->format_to_str = vsprintf;
}
