/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: helper functions for communication with the SD card
 */

#include "sd.h"

char send_data_byte(char byte) {
    while (REGW(SPI_BASE, SPI_TXDATA) & (1 << 31));
    if (earth->platform == ARTY)
        REGB(SPI_BASE, SPI_TXDATA) = byte;
    else /* QEMU */
        REGW(SPI_BASE, SPI_TXDATA) = byte;

    uint rxdata;
    while ((rxdata = REGW(SPI_BASE, SPI_RXDATA)) & (1 << 31));
    return (char)(rxdata & 0xFF);
}

char recv_data_byte() { return send_data_byte(0xFF); }

char sd_exec_cmd(char* cmd) {
    for (uint i = 0; i < 6; i++) send_data_byte(cmd[i]);

    for (uint reply, i = 0; i < 8000; i++)
        if ((reply = recv_data_byte()) != 0xFF) return reply;

    FATAL("SD card not responding cmd%d", cmd[0] ^ 0x40);
}

char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (recv_data_byte() != 0xFF);
    sd_exec_cmd(cmd55);

    while (recv_data_byte() != 0xFF);
    return sd_exec_cmd(cmd);
}

static void spi_set_clock(long baud_rate) {
    long div = (CPU_CLOCK_RATE / (2 * baud_rate)) - 1;
    REGW(SPI_BASE, SPI_SCKDIV) = (div & 0xFFF);
}

enum sd_type SD_CARD_TYPE = SD_TYPE_UNKNOWN;

void sdinit() {
    /* Configure the SPI controller */
    spi_set_clock(100000);
    REGW(SPI_BASE, SPI_CSMODE) = 1;
    REGW(SPI_BASE, SPI_CSDEF) = 0;
    REGW(SPI_BASE, SPI_FCTRL) = 0;

    INFO("Set CS and MOSI to 1 and toggle clock.");
    uint i, rxdata;
    for (i = 0; i < 1000; i++) send_data_byte(0xFF);
    /* Keep chip select line low */
    REGW(SPI_BASE, SPI_CSDEF) = 1;
    for (i = 0; i < 200000; i++);

    INFO("Set CS to 0 and send cmd0 to SD card.");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(cmd0);
    while (reply != 0x01) reply = recv_data_byte();
    while (recv_data_byte() != 0xFF);

    INFO("Set SPI clock frequency to %ldHz", CPU_CLOCK_RATE / 4);
    spi_set_clock(CPU_CLOCK_RATE / 4);

    INFO("Check SD card type and voltage with cmd8");
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    reply = sd_exec_cmd(cmd8);

    if (reply & 0x04) {
        /* Illegal command */
        SD_CARD_TYPE = SD_TYPE_SD1;
    } else {
        /* Only need the last byte of r7 response */
        uint payload;
        for (uint i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = recv_data_byte();
        INFO("SD card replies cmd8 with status 0x%.2x and payload 0x%.8x", reply, payload);

        if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");
        SD_CARD_TYPE = SD_TYPE_SD2;
    }
    while (recv_data_byte() != 0xFF);

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (recv_data_byte() != 0xFF);

    INFO("Set block size to 512 bytes with cmd16");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    reply = sd_exec_cmd(cmd16);
    while (recv_data_byte() != 0xFF);

    if (SD_CARD_TYPE == SD_TYPE_SD2) {
        INFO("Check SD card capacity with cmd58");

        char reply, payload[4], cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
        sd_exec_cmd(cmd58);
        for (uint i = 0; i < 4; i++) payload[3 - i] = recv_data_byte();

        if ((payload[3] & 0xC0) == 0xC0) SD_CARD_TYPE = SD_TYPE_SDHC;
        INFO("SD card replies cmd58 with payload 0x%.8x", *(int*)payload);

        while (recv_data_byte() != 0xFF);
    }
}
