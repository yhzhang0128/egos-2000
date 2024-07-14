/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: definitions for UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#include "egos.h"

#define SIFIVE_UART_TXDATA  0UL
#define SIFIVE_UART_RXDATA  4UL

#define LITEX_UART_TXFULL   4UL
#define LITEX_UART_RXEMPTY  8UL
#define LITEX_UART_EVPEND   16UL

void uart_putc(int c) {
    if (earth->platform == ARTY) {
        while (REGW(UART_BASE, LITEX_UART_TXFULL));
        REGW(UART_BASE, 0) = c;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 1;
    } else {
        while ((REGW(UART_BASE, SIFIVE_UART_TXDATA) & (1 << 31)));
        REGW(UART_BASE, SIFIVE_UART_TXDATA) = c;
    }
}

void uart_getc(int* c) {
    if (earth->platform == ARTY) {
        while(REGW(UART_BASE, LITEX_UART_RXEMPTY));
        *c = REGW(UART_BASE, 0) & 0xFF;
        REGW(UART_BASE, LITEX_UART_EVPEND) = 2;
    } else {
        while ((*c = REGW(UART_BASE, SIFIVE_UART_RXDATA)) & (1 << 31));
        *c &= 0xFF;
    }
}
