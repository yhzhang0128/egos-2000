/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the SD card
 * Only SDHC and SDXC cards are supported
 */

#include "sd.h"

enum {
      SD_TYPE_SD1,
      SD_TYPE_SD2,
      SD_TYPE_SDHC,
      SD_TYPE_UNKNOWN
};
static int SD_CARD_TYPE = SD_TYPE_UNKNOWN;

static void sd_check_capacity() {
    INFO("Check SD card capacity with cmd58");
    while (recv_data_byte() != 0xFF);

    char reply, payload[4], cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
    if (sd_exec_cmd(cmd58)) FATAL("SD card cmd58 fails");
    for (int i = 0; i < 4; i++) payload[3 - i] = recv_data_byte();

    if ((payload[3] & 0xC0) == 0xC0) SD_CARD_TYPE = SD_TYPE_SDHC;
    INFO("SD card replies cmd58 with payload 0x%.8x", *(int*)payload);

    while (recv_data_byte() != 0xFF);
}

static int sd_check_type() {
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    char reply = sd_exec_cmd(cmd8);

    INFO("SD card replies cmd8 with status 0x%.2x", reply);
    if (reply & 0x04) {
        /* Illegal command */
        SD_CARD_TYPE = SD_TYPE_SD1;
    } else {
        /* Only need last byte of r7 response */
        unsigned long payload;
        for (int i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = recv_data_byte();
        INFO("SD card replies cmd8 with payload 0x%.8x", payload);

        if ((payload & 0xFFF) != 0x1AA) return -1;
        SD_CARD_TYPE = SD_TYPE_SD2;
    }

    while (recv_data_byte() != 0xFF);
    return 0;
}

static void sd_reset() {
    /* Keep chip select line high */
    INFO("Set CS and MOSI to 1 and toggle clock.");
    REGW(SPI1_BASE, SPI1_CSMODE) = 2;

    unsigned long i, rxdata;
    for (i = 0; i < 1000; i++) send_data_byte(0xFF);

    /* Keep chip select line low */
    REGW(SPI1_BASE, SPI1_CSDEF) = 1;
    for (i = 0; i < 200000; i++);
    
    INFO("Set CS to 0 and send cmd0 to SD card.");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(cmd0);

    INFO("Wait for SD card's reply to cmd0 (QEMU will stuck here)");
    while (reply != 0x01) reply = recv_data_byte();
    while (recv_data_byte() != 0xFF);
}

static void spi_config() {
    /* Set phase as 0*/
    /* Set polarity as 0 */
    REGW(SPI1_BASE, SPI1_SCKMODE) = 0;

    /* Set CS line */
    REGW(SPI1_BASE, SPI1_CSID) = 0;

    /* Set CS active-high as 0 */
    REGW(SPI1_BASE, SPI1_CSDEF) = 0;

    /* Toggle off memory-mapped SPI flash mode;
     * toggle on programmable IO mode */
    REGW(SPI1_BASE, SPI1_FCTRL) = 0;

    /* Set protocol as SPI_SINGLE */
    /* Set endianness as 0 */
    /* Always populate receive FIFO */
    /* Set frame length */
    REGW(SPI1_BASE, SPI1_FMT) = 0x80000;
}

static void spi_set_clock(long baud_rate) {
    long div = (CPU_CLOCK_RATE / (2 * baud_rate)) - 1;
    REGW(SPI1_BASE, SPI1_SCKDIV) = (div & 0xFFF);
}

void sdinit() {
    spi_set_clock(100000);
    spi_config();

    sd_reset();
    INFO("Set SPI clock frequency to %ldHz", CPU_CLOCK_RATE / 4);
    spi_set_clock(CPU_CLOCK_RATE / 4);

    INFO("Check SD card type and voltage with cmd8");
    if (0 != sd_check_type()) FATAL("Fail to check SD card type");

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (recv_data_byte() != 0xFF);

    INFO("Set block size to 512 bytes with cmd16");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    char reply = sd_exec_cmd(cmd16);
    while (recv_data_byte() != 0xFF);

    if (SD_CARD_TYPE == SD_TYPE_SD2) sd_check_capacity();
    if (SD_CARD_TYPE != SD_TYPE_SDHC) FATAL("Only SDHC/SDXC supported");
}
