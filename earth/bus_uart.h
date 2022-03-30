/* 
 * Definitions for controlling UART in FE310
 * copied from the Freedom Metal library: 
 * https://github.com/sifive/freedom-metal
 */
#pragma once

#define UART_BASE_ADDR            0x10013000UL

#define UART_RXEMPTY              (1 << 31)
#define METAL_SIFIVE_UART0_RXDATA 4UL
#define METAL_SIFIVE_UART0_TXCTRL 8UL
#define METAL_SIFIVE_UART0_RXCTRL 12UL
#define METAL_SIFIVE_UART0_DIV    24UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define UART_REG(offset) (UART_BASE_ADDR + offset)
#define UART_REGW(offset) (__METAL_ACCESS_ONCE((unsigned int*)UART_REG(offset)))
