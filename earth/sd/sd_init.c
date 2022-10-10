/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the SD card
 * Only SDHC and SDXC cards are supported.
 */

#include "sd.h"

static void sd_spi_reset();
static void sd_spi_config();
static void sd_spi_set_clock(long baud_rate);

static int sd_check_type();
static void sd_check_capacity();

int SD_CARD_TYPE = SD_CARD_TYPE_UNKNOWN;

void sdinit() {
    sd_spi_set_clock(100000);
    sd_spi_config();
    sd_spi_reset();

    INFO("Set SPI clock frequency to %ldHz", CPU_CLOCK_RATE / 4);
    sd_spi_set_clock(CPU_CLOCK_RATE / 4);

    INFO("Check SD card type and voltage with cmd8");
    if (0 != sd_check_type()) FATAL("Fail to check SD card type");

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_CARD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (recv_data_byte() != 0xFF);

    INFO("Set block size to 512 bytes with cmd16");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    char reply = sd_exec_cmd(cmd16);
    while (recv_data_byte() != 0xFF);

    if (SD_CARD_TYPE == SD_CARD_TYPE_SD2) sd_check_capacity();

    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        FATAL("Only SDHC/SDXC cards are supported");

    INFO("SD card is high capacity SDHC/SDXC card");
}

static int sd_check_type() {
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    char reply = sd_exec_cmd(cmd8);

    INFO("SD card replies cmd8 with status 0x%.2x", reply);
    if (reply & 0x04) {
        /* Illegal command */
        SD_CARD_TYPE = SD_CARD_TYPE_SD1;
    } else {
        /* only need last byte of r7 response */
        unsigned long payload;
        for (int i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = recv_data_byte();
        INFO("SD card replies cmd8 with payload 0x%.8x", payload);

        if ((payload & 0xFFF) != 0x1AA) return -1;
        SD_CARD_TYPE = SD_CARD_TYPE_SD2;
    }

    while (recv_data_byte() != 0xFF);
    return 0;
}

static void sd_check_capacity() {
    INFO("Check SD card capacity with cmd58");
    while (recv_data_byte() != 0xFF);

    char reply, cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
    if (sd_exec_cmd(cmd58)) FATAL("cmd58 returns failure");
    INFO("SD card replies cmd58 with status 0x00");

    unsigned long payload;
    for (int i = 0; i < 4; i++) {
        reply = ((char*)&payload)[3 - i] = recv_data_byte();

        if (i == 0 && ((reply & 0XC0) == 0XC0))
            SD_CARD_TYPE = SD_CARD_TYPE_SDHC;
    }
    INFO("SD card replies cmd58 with payload 0x%.8x", payload);

    while (recv_data_byte() != 0xFF);
}

static void sd_spi_reset() {
    INFO("Set CS and MOSI to 1 and toggle clock.");

    /* Keep chip select line high */
    REGW(SPI1_BASE, SPI1_CSMODE) &= ~3;
    REGW(SPI1_BASE, SPI1_CSMODE) |= 2;

    unsigned long i, rxdata;
    for (i = 0; i < 1000; i++) send_data_byte(0xFF);

    /* Keep chip select line low */
    REGW(SPI1_BASE, SPI1_CSDEF) = 1;
    REGW(SPI1_BASE, SPI1_CSMODE) &= ~3;
    REGW(SPI1_BASE, SPI1_CSMODE) |= 2;
    for (i = 0; i < 200000; i++);
    
    INFO("Set CS to 0 and send cmd0 through MOSI.");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(cmd0);

    while (reply != 0x01) reply = recv_data_byte();
    INFO("SD card replies cmd0 with 0x01");

    while (recv_data_byte() != 0xFF);
}

static void sd_spi_config() {
    /* Set protocol as SPI_SINGLE */
    REGW(SPI1_BASE, SPI1_FMT) &= ~3;

    /* Set phase as 0*/
    REGW(SPI1_BASE, SPI1_SCKMODE) &= ~1;

    /* Set polarity as 0 */
    REGW(SPI1_BASE, SPI1_SCKMODE) &= ~2;

    /* Set endianness as 0 */
    REGW(SPI1_BASE, SPI1_FMT) &= ~4;

    /* Always populate receive FIFO */
    REGW(SPI1_BASE, SPI1_FMT) &= ~8;

    /* Set CS active-high as 0 */
    REGW(SPI1_BASE, SPI1_CSDEF) = 0;

    /* Set frame length */
    if ((REGW(SPI1_BASE, SPI1_FMT) & 0xF0000) != 0x80000) {
        REGW(SPI1_BASE, SPI1_FMT) &= ~0xF0000;
        REGW(SPI1_BASE, SPI1_FMT) |= 0x80000;
    }

    /* Set CS line */
    REGW(SPI1_BASE, SPI1_CSID) = 0;

    /* Toggle off memory-mapped SPI flash mode;
     * toggle on programmable IO mode */
    REGW(SPI1_BASE, SPI1_FCTRL) = 0;
}

static void sd_spi_set_clock(long baud_rate) {
    long div = (CPU_CLOCK_RATE / (2 * baud_rate)) - 1;

    REGW(SPI1_BASE, SPI1_SCKDIV) &= ~0xFFF;
    REGW(SPI1_BASE, SPI1_SCKDIV) |= (div & 0xFFF);
}
