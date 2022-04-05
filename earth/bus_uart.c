/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* 
 * definitions for controlling UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#define UART0_BASE    0x10013000UL

#define UART0_TXDATA  0UL
#define UART0_RXDATA  4UL
#define UART0_TXCTRL  8UL
#define UART0_RXCTRL  12UL
#define UART0_DIV     24UL

#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define UART_REG(offset) (UART0_BASE + offset)
#define UART_REGW(offset) (ACCESS((unsigned int*)UART_REG(offset)))

static void uart_init(long baud_rate) {
    UART_REGW(UART0_DIV) = CPU_CLOCK_RATE / baud_rate - 1;
    UART_REGW(UART0_TXCTRL) |= 1;
    UART_REGW(UART0_RXCTRL) |= 1;
}

static int uart_getc(int* c) {
    int ch = UART_REGW(UART0_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0xFF);
}

static void uart_putc(int c) {
    while ((UART_REGW(UART0_TXDATA) & (1 << 31)));
    UART_REGW(UART0_TXDATA) = c;
}
