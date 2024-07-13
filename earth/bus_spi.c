/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: definitions for SPI1 in FE310
 */
#include "egos.h"

#define SPI_SCKDIV   0UL
#define SPI_TXDATA   72UL
#define SPI_RXDATA   76UL

char spi_putc(char byte) {
    while (REGW(SPI_BASE, SPI_TXDATA) & (1 << 31));
    if (earth->platform == ARTY)
        REGB(SPI_BASE, SPI_TXDATA) = byte;
    else /* QEMU */
        REGW(SPI_BASE, SPI_TXDATA) = byte;

    uint rxdata;
    while ((rxdata = REGW(SPI_BASE, SPI_RXDATA)) & (1 << 31));
    return (char)(rxdata & 0xFF);
}

char spi_getc() { return spi_putc(0xFF); }
