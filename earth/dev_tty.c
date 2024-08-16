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

#define LITEX_UART_TXFULL   4UL
#define LITEX_UART_RXEMPTY  8UL
#define LITEX_UART_EVPEND   16UL

#define SIFIVE_UART_TXDATA  0UL
#define SIFIVE_UART_RXDATA  4UL

void uart_putc(char c) {
    if (earth->platform == ARTY) {
        while (REGW(UART_BASE, LITEX_UART_TXFULL));
        REGW(UART_BASE, 0) = c;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 1;
    } else {
        while ((REGW(UART_BASE, SIFIVE_UART_TXDATA) & (1 << 31)));
        REGW(UART_BASE, SIFIVE_UART_TXDATA) = c;
    }
}

void uart_getc(char* c) {
    if (earth->platform == ARTY) {
        while(REGW(UART_BASE, LITEX_UART_RXEMPTY));
        *c = REGW(UART_BASE, 0) & 0xFF;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 2;
    } else {
        int ch;
        while ((ch = REGW(UART_BASE, SIFIVE_UART_RXDATA)) & (1 << 31));
        *c = ch & 0xFF;
    }
}

int tty_write(char* buf, uint len) {
    for (uint i = 0; i < len; i++) uart_putc(buf[i]);
    return len;
}

void tty_init() {
    earth->tty_read = uart_getc;
    earth->tty_write = tty_write;
    earth->format_to_str = vsprintf;
}
