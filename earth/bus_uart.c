/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: definitions for UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#include "egos.h"
#include "bus_gpio.c"

#define UART0_BASE    0x10013000UL
#define UART0_TXDATA  0UL
#define UART0_RXDATA  4UL
#define UART0_TXCTRL  8UL
#define UART0_RXCTRL  12UL
#define UART0_DIV     24UL

void uart_init(long baud_rate) {
    REGW(UART0_BASE, UART0_DIV) = CPU_CLOCK_RATE / baud_rate - 1;
    REGW(UART0_BASE, UART0_TXCTRL) |= 1;
    REGW(UART0_BASE, UART0_RXCTRL) |= 1;

    /* UART0 send/recv are mapped to GPIO pin16 and pin17 */
    REGW(GPIO0_BASE, GPIO0_IOF_ENABLE) |= (1 << 16) | (1 << 17);
}

int uart_getc(int* c) {
    int ch = REGW(UART0_BASE, UART0_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0xFF);
}

void uart_putc(int c) {
    while ((REGW(UART0_BASE, UART0_TXDATA) & (1 << 31)));
    REGW(UART0_BASE, UART0_TXDATA) = c;
}
