/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: definitions for UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#include "egos.h"

#define UART0_TXDATA  0UL
#define UART0_RXDATA  4UL
#define UART0_TXCTRL  8UL
#define UART0_RXCTRL  12UL
#define UART0_DIV     24UL

void uart_putc(int c) {
    while ((REGW(UART0_BASE, UART0_TXDATA) & (1 << 31)));
    REGW(UART0_BASE, UART0_TXDATA) = c;
}

int uart_getc(int* c) {
    uint ch = REGW(UART0_BASE, UART0_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0xFF);
}
