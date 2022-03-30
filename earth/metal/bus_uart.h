/* 
 * Definitions for controlling UART in FE310
 * copied from the Freedom Metal library: https://github.com/sifive/freedom-metal
 */
#pragma once

struct metal_uart;
struct metal_uart_vtable {
    void (*init)(struct metal_uart *uart, int baud_rate);
};

struct metal_uart {
    const struct metal_uart_vtable *vtable;
};

struct metal_uart *metal_uart_get_device(unsigned int device_num);

__inline__ void metal_uart_init(struct metal_uart *uart, int baud_rate) {
    uart->vtable->init(uart, baud_rate);
}

#define UART_RXEMPTY (1 << 31)
#define METAL_SIFIVE_UART0_RXDATA 4UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define UART_REG(offset) (((unsigned long)control_base + offset))
#define UART_REGW(offset) (__METAL_ACCESS_ONCE((unsigned int*)UART_REG(offset)))
