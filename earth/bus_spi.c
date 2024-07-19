/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: definitions for SPI1 in FE310
 */
#include "egos.h"

#define SIFIVE_SPI_TXDATA   72UL
#define SIFIVE_SPI_RXDATA   76UL

#define LITEX_SPI_CONTROL    0UL
#define LITEX_SPI_STATUS     4UL
#define LITEX_SPI_MOSI       8UL
#define LITEX_SPI_MISO      12UL
#define LITEX_SPI_CLKDIV    24UL

char spi_transfer(char byte) {
    /* SPI transfer means sending a byte and then receiving a byte */
    uint rxdata;
    if (earth->platform == ARTY) {
        REGW(SPI_BASE, LITEX_SPI_MOSI) = byte;
        REGW(SPI_BASE, LITEX_SPI_CONTROL) = (8 * (1 << 8) | (1));
        while ((REGW(SPI_BASE, LITEX_SPI_STATUS) & 1) != 1);
        rxdata = REGW(SPI_BASE, LITEX_SPI_MISO);
    } else {
        while (REGW(SPI_BASE, SIFIVE_SPI_TXDATA) & (1 << 31));
        REGW(SPI_BASE, SIFIVE_SPI_TXDATA) = byte;
        while ((rxdata = REGW(SPI_BASE, SIFIVE_SPI_RXDATA)) & (1 << 31));
    }
    return (char)(rxdata & 0xFF);
}

void spi_set_clock(uint freq) {
    #define CPU_CLOCK_RATE 100000000  /* 100MHz */
    uint div = CPU_CLOCK_RATE / freq + 1;
    REGW(SPI_BASE, LITEX_SPI_CLKDIV) = div;
}
