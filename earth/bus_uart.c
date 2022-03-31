/* 
 * Definitions for controlling UART in FE310
 * copied from the Freedom Metal library: 
 * https://github.com/sifive/freedom-metal
 */

#define CPU_CLOCK_RATE            65000000
#define UART_BASE_ADDR            0x10013000UL

#define METAL_SIFIVE_UART0_DIV    24UL
#define METAL_SIFIVE_UART0_TXCTRL 8UL
#define METAL_SIFIVE_UART0_RXCTRL 12UL
#define METAL_SIFIVE_UART0_RXDATA 4UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define UART_REG(offset) (UART_BASE_ADDR + offset)
#define UART_REGW(offset) (__METAL_ACCESS_ONCE((unsigned int*)UART_REG(offset)))

static void uart_set_clock(long baud_rate) {
    UART_REGW(METAL_SIFIVE_UART0_DIV) = CPU_CLOCK_RATE / baud_rate - 1;
    UART_REGW(METAL_SIFIVE_UART0_TXCTRL) |= 1;
    UART_REGW(METAL_SIFIVE_UART0_RXCTRL) |= 1;
}

static int uart_getc(int* c) {
    int ch = UART_REGW(METAL_SIFIVE_UART0_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0x0ff);
}
