/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple TTY device driver
 */

#include "egos.h"

#define LITEX_UART_TXFULL  4UL
#define LITEX_UART_RXEMPTY 8UL
#define LITEX_UART_EVPEND  16UL

#define SIFIVE_UART_TXDATA 0UL
#define SIFIVE_UART_RXDATA 4UL
#define SIFIVE_UART_IP     20UL

uint uart_rx_empty() {
    return (earth->platform == HARDWARE)
               ? REGW(UART_BASE, LITEX_UART_RXEMPTY)
               : !(REGW(UART_BASE, SIFIVE_UART_IP) & 2);
}

void uart_getc(char* c) {
    if (earth->platform == HARDWARE) {
        while (REGW(UART_BASE, LITEX_UART_RXEMPTY));
        *c                                 = REGW(UART_BASE, 0) & 0xFF;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 2;
    } else {
        int ch;
        while ((ch = REGW(UART_BASE, SIFIVE_UART_RXDATA)) & (1 << 31));
        *c = ch & 0xFF;
    }
}

void uart_putc(char c) {
    if (earth->platform == HARDWARE) {
        while (REGW(UART_BASE, LITEX_UART_TXFULL));
        REGW(UART_BASE, 0)                 = c;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 1;
    } else {
        while ((REGW(UART_BASE, SIFIVE_UART_TXDATA) & (1 << 31)));
        REGW(UART_BASE, SIFIVE_UART_TXDATA) = c;
    }
}

void tty_init() {
    earth->tty_read        = uart_getc;
    earth->tty_write       = uart_putc;
    earth->tty_input_empty = uart_rx_empty;
}
