/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* 
 * definitions for controlling UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#define GPIO0_BASE    0x10012000UL
#define GPIO0_IOF_EN  56UL

#define GPIO_REG(offset) (GPIO0_BASE + offset)
#define GPIO_REGW(offset) (ACCESS((unsigned int*)GPIO_REG(offset)))

#define UART0_BASE    0x10013000UL
#define UART0_TXDATA  0UL
#define UART0_RXDATA  4UL
#define UART0_TXCTRL  8UL
#define UART0_RXCTRL  12UL
#define UART0_DIV     24UL

#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define UART_REG(offset) (UART0_BASE + offset)
#define UART_REGW(offset) (ACCESS((unsigned int*)UART_REG(offset)))


void uart_init(long baud_rate) {
    UART_REGW(UART0_DIV) = CPU_CLOCK_RATE / baud_rate - 1;
    UART_REGW(UART0_TXCTRL) |= 1;
    UART_REGW(UART0_RXCTRL) |= 1;

    /* UART0 maps to GPIO pin16 and pin17 on FE310 */
    GPIO_REGW(GPIO0_IOF_EN) |= 0x30000;
}

int uart_getc(int* c) {
    int ch = UART_REGW(UART0_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0xFF);
}

void uart_putc(int c) {
    while ((UART_REGW(UART0_TXDATA) & (1 << 31)));
    UART_REGW(UART0_TXDATA) = c;
}
