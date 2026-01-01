/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: a simple TTY device driver
 */

#include "egos.h"

#define LITEX_UART_TXFULL  4UL
#define LITEX_UART_RXEMPTY 8UL
#define LITEX_UART_EVPEND  16UL
#define VIRT_LINE_STATUS   5UL

uint uart_rx_empty() {
    return (earth->platform == HARDWARE)
               ? REGW(UART_BASE, LITEX_UART_RXEMPTY)
               : !(REGB(UART_BASE, VIRT_LINE_STATUS) & (1 << 0));
}

void uart_getc(char* c) {
    while (uart_rx_empty());
    if (earth->platform == HARDWARE) {
        *c                                 = REGW(UART_BASE, 0) & 0xFF;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 2;
    } else {
        *c = REGW(UART_BASE, 0) & 0xFF;
    }
}

void uart_putc(char c) {
    if (earth->platform == HARDWARE) {
        while (REGW(UART_BASE, LITEX_UART_TXFULL));
        REGW(UART_BASE, 0)                 = c;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 1;
    } else {
        while (!(REGB(UART_BASE, VIRT_LINE_STATUS) & (1 << 5)));
        REGW(UART_BASE, 0) = c;
    }
}

void tty_init() {
    earth->tty_read        = uart_getc;
    earth->tty_write       = uart_putc;
    earth->tty_input_empty = uart_rx_empty;
}
