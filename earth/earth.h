#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "elf.h"
#include "mmu.h"

int tty_init();
int tty_read(char* buf, int len);
int tty_write(const char *format, ...);

int disk_init();
int disk_busy();
int disk_read(int block_no, int nblocks, char* dst);
int disk_write(int block_no, int nblocks, char* src);

typedef void (*handler_t)(int);
int intr_init();
int intr_enable();
int intr_disable();
int intr_register(handler_t handler);

int mmu_init();
int mmu_alloc(int* frame_no, int* addr);
int mmu_map(int pid, int page_no, int frame_no, int flag);
int mmu_switch(int pid);


/* definitions for controlling UART in tty, copied from metal/uart.h */
struct metal_uart;
struct metal_uart_vtable {
    void (*init)(struct metal_uart *uart, int baud_rate);
    int (*putc)(struct metal_uart *uart, int c);
    int (*txready)(struct metal_uart *uart);
    int (*getc)(struct metal_uart *uart, int *c);
    int (*get_baud_rate)(struct metal_uart *uart);
    int (*set_baud_rate)(struct metal_uart *uart, int baud_rate);
    struct metal_interrupt *(*controller_interrupt)(struct metal_uart *uart);
    int (*get_interrupt_id)(struct metal_uart *uart);
    int (*tx_interrupt_enable)(struct metal_uart *uart);
    int (*tx_interrupt_disable)(struct metal_uart *uart);
    int (*rx_interrupt_enable)(struct metal_uart *uart);
    int (*rx_interrupt_disable)(struct metal_uart *uart);
    int (*set_tx_watermark)(struct metal_uart *uart, size_t length);
    size_t (*get_tx_watermark)(struct metal_uart *uart);
    int (*set_rx_watermark)(struct metal_uart *uart, size_t length);
    size_t (*get_rx_watermark)(struct metal_uart *uart);
};

struct metal_uart {
    const struct metal_uart_vtable *vtable;
};

struct metal_uart *metal_uart_get_device(unsigned int device_num);
__inline__ void metal_uart_init(struct metal_uart *uart, int baud_rate) {
    uart->vtable->init(uart, baud_rate);
}

__inline__ int metal_uart_getc(struct metal_uart *uart, int *c) {
    return uart->vtable->getc(uart, c);
}
