/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include <string.h>

enum disk_type {
      SD_CARD,
      FLASH_ROM
};
static enum disk_type type;

static void sdinit();
static int sdread(uint offset, uint nblock, char* dst);
static int sdwrite(uint offset, uint nblock, char* src);

static void disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == SD_CARD) {
        sdread(block_no, nblocks, dst);
    } else {
        char* src = (char*)0x20400000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    }
}

static void disk_write(uint block_no, uint nblocks, char* src) {
    if (type == FLASH_ROM) FATAL("disk_write: Writing to ROM");
    sdwrite(block_no, nblocks, src);
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

    type = (earth->platform == ARTY)? FLASH_ROM : SD_CARD;
    if (earth->platform == QEMU) sdinit();
}

enum sd_type {
      SD_TYPE_SD1,
      SD_TYPE_SD2,
      SD_TYPE_SDHC,
      SD_TYPE_UNKNOWN
};
enum sd_type SD_CARD_TYPE = SD_TYPE_UNKNOWN;

char spi_getc();
char spi_putc(char);

static char sd_exec_cmd(char* cmd) {
    for (uint i = 0; i < 6; i++) spi_putc(cmd[i]);

    for (uint reply, i = 0; i < 8000; i++)
        if ((reply = spi_getc()) != 0xFF) return reply;

    FATAL("SD card not responding cmd%d", cmd[0] ^ 0x40);
}

static char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (spi_getc() != 0xFF);
    sd_exec_cmd(cmd55);

    while (spi_getc() != 0xFF);
    return sd_exec_cmd(cmd);
}


static void single_read(uint offset, char* dst) {
    if (SD_CARD_TYPE == SD_TYPE_SD2) offset = offset * BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_getc() != 0xFF);

    /* Send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(cmd17))
        FATAL("SD card replies cmd17 with status 0x%.2x", reply);

    /* Wait for the data packet and ignore the 2-byte checksum */
    while (spi_getc() != 0xFE);
    for (uint i = 0; i < BLOCK_SIZE; i++) dst[i] = spi_getc();
    spi_getc();
    spi_getc();
}

static void single_write(uint offset, char* src) {
    if (SD_CARD_TYPE == SD_TYPE_SD2) offset = offset * BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_getc() != 0xFF);

    /* Send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* Send data packet: token + block + dummy 2-byte checksum */
    spi_putc(0xFE);
    for (uint i = 0; i < BLOCK_SIZE; i++) spi_putc(src[i]);
    spi_putc(0xFF);
    spi_putc(0xFF);

    /* Wait for SD card ack of data packet */
    while ((reply = spi_getc()) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card write ack with status 0x%.2x", reply);
}

static int sdread(uint offset, uint nblock, char* dst) {
    /* A better way to read multiple blocks using SD card
     * command 18 is left to students as a course project */

    for (uint i = 0; i < nblock; i++)
        single_read(offset + i, dst + BLOCK_SIZE * i);
    return 0;
}

static int sdwrite(uint offset, uint nblock, char* src) {
    /* A better way to write multiple blocks using SD card
     * command 25 is left to students as a course project */

    for (uint i = 0; i < nblock; i++)
        single_write(offset + i, src + BLOCK_SIZE * i);
    return 0;
}

static void sdinit() {
    /* Configure the SPI controller */
    #define SPI_CSMODE   24UL
    #define SPI_CSDEF    20UL
    REGW(SPI_BASE, SPI_CSMODE) = 1;
    REGW(SPI_BASE, SPI_CSDEF)  = 0;

    INFO("Set CS and MOSI to 1 and toggle clock.");
    for (uint i = 0; i < 1000; i++) spi_putc(0xFF);
    /* Keep chip select line low */
    REGW(SPI_BASE, SPI_CSDEF) = 1;

    INFO("Set CS to 0 and send cmd0 to SD card.");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(cmd0);
    while (reply != 0x01) reply = spi_getc();
    while (spi_getc() != 0xFF);

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
            ((char*)&payload)[3 - i] = spi_getc();
        INFO("SD card replies cmd8 with status 0x%.2x and payload 0x%.8x", reply, payload);

        if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");
        SD_CARD_TYPE = SD_TYPE_SD2;
    }
    while (spi_getc() != 0xFF);

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (spi_getc() != 0xFF);

    INFO("Set block size to 512 bytes with cmd16");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    reply = sd_exec_cmd(cmd16);
    while (spi_getc() != 0xFF);

    if (SD_CARD_TYPE == SD_TYPE_SD2) {
        INFO("Check SD card capacity with cmd58");

        char reply, payload[4], cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
        sd_exec_cmd(cmd58);
        for (uint i = 0; i < 4; i++) payload[3 - i] = spi_getc();

        if ((payload[3] & 0xC0) == 0xC0) SD_CARD_TYPE = SD_TYPE_SDHC;
        INFO("SD card replies cmd58 with payload 0x%.8x", *(int*)payload);

        while (spi_getc() != 0xFF);
    }
}
