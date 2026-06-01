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

/*
when receiving a byte, bit0 of the register at UART_BASE + VIRT_LINE_STATUS is set to 1, and the byte can be read from UART_BASE

when sending a byte, uart_putc waits for UART to be idle, by checking bit5 of the line register
- after UART is ready, it will write to UART_BASE the byte to be printed on the terminal

Memory Mapped IO
- hardware can define special areas in memory used to contorl IO devices
- based on if ran on RISCV or QEMU, UART_BASE is mapped to different addresses, but the same code can be used to read/write UART registers

SPI and SD card
- computer needs a disk storing blocks of data when the computer is off, (in this case, SD Card)
- in RISCV, SD card is connected with CPU using Serial Peripheral Interface (SPI), which is a simple protocol to send commands and data between CPU and SD card
    - this has 4 hardware pins

CPU is SPI Main, SD card is SPI Sub
- Chip Select (CS) resets the SD card before starting to use it.
- Serial Clock (SCLK) provides clock signals from the CPU (e.g., 20MHz).
- Main Out Sub In (MOSI) is used by the CPU to send bytes to the SD card.
- Main In Sub Out (MISO) is used by the SD card to send bytes to the CPU.

SPI Main and SPI sub EXCHANGE bytes during communication

SPI is synchronous (always receives a byte after sending a byte)
- spi_exchange uses MOSI and MISO pins to send a byte and receive a byte at the same time, and uses SCLK to synchronize the communication between CPU and SD card

*/

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
