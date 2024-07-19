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

static int sd_init();
static void sd_read(uint offset, char* dst);
static void sd_write(uint offset, char* src);

static void disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == SD_CARD) {
        for (uint i = 0; i < nblocks; i++)
            sd_read(block_no + i, dst + BLOCK_SIZE * i);
    } else {
        char* src = (char*)0x20400000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    }
}

static void disk_write(uint block_no, uint nblocks, char* src) {
    if (type == FLASH_ROM) FATAL("disk_write: Writing to ROM");
    for (uint i = 0; i < nblocks; i++)
        sd_write(block_no + i, src + BLOCK_SIZE * i);
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

    type = SD_CARD;
    if (sd_init() < 0) {
        type = FLASH_ROM;
        CRITICAL("Failed at initializing SD and use FLASH ROM instead");
    }
}

char spi_transfer(char);
char spi_set_clock(uint);

static char sd_exec_cmd(char* cmd) {
    for (uint i = 0; i < 6; i++) spi_transfer(cmd[i]);

    for (uint reply, i = 0; i < 8000; i++)
        if ((reply = spi_transfer(0xFF)) != 0xFF) return reply;

    return 0xFF;
}

static char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (spi_transfer(0xFF) != 0xFF);
    sd_exec_cmd(cmd55);

    while (spi_transfer(0xFF) != 0xFF);
    return sd_exec_cmd(cmd);
}


static void sd_read(uint offset, char* dst) {
    /* QEMU uses SD2 while Arty uses SDHC/SDXC */
    if (earth->platform == QEMU) offset *= BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_transfer(0xFF) != 0xFF);

    /* Send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(cmd17))
        FATAL("SD card replies cmd17 with status 0x%.2x", reply);

    /* Wait for the data packet and ignore the 2-byte checksum */
    while (spi_transfer(0xFF) != 0xFE);
    for (uint i = 0; i < BLOCK_SIZE; i++) dst[i] = spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);
}

static void sd_write(uint offset, char* src) {
    /* QEMU uses SD2 while Arty uses SDHC/SDXC */
    if (earth->platform == QEMU) offset *= BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_transfer(0xFF) != 0xFF);

    /* Send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* Send data packet: token + block + dummy 2-byte checksum */
    spi_transfer(0xFE);
    for (uint i = 0; i < BLOCK_SIZE; i++) spi_transfer(src[i]);
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    /* Wait for SD card ack of data packet */
    while ((reply = spi_transfer(0xFF)) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card write ack with status 0x%.2x", reply);
}

static int sd_init() {
    /* Configure the SPI controller */
    INFO("Set CS to HIGH and toggle clock.");

    if (earth->platform == ARTY) {
        spi_set_clock(400000);
        #define SPI_CS 16UL
        REGW(SPI_BASE, SPI_CS) = 0;
        for (uint i = 0; i < 1000; i++) spi_transfer(0xFF);
        REGW(SPI_BASE, SPI_CS) = 1;
    } else {
        #define SPI_CSMODE   24UL
        #define SPI_CSDEF    20UL
        REGW(SPI_BASE, SPI_CSMODE) = 1;
        for (uint i = 0; i < 1000; i++) spi_transfer(0xFF);
        /* Keep chip select line low */
        REGW(SPI_BASE, SPI_CSDEF) = 1;
    }

    INFO("Set CS to LOW and send cmd0 to SD card.");
    char reply, cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    if ((reply = sd_exec_cmd(cmd0)) == 0xFF) return -1;
    while (reply != 0x01) reply = spi_transfer(0xFF);
    while (spi_transfer(0xFF) != 0xFF);

    INFO("Check SD card type and voltage with cmd8");
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    reply = sd_exec_cmd(cmd8);

    if (reply & 0x04) {
        /* Illegal command */
        FATAL("Only SD2/SDHC/SDXC are supported");
    } else {
        /* Only need the last byte of r7 response */
        uint payload;
        for (uint i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = spi_transfer(0xFF);
        INFO("SD card replies cmd8 with status 0x%.2x and payload 0x%.8x", reply, payload);

        if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");
    }
    while (spi_transfer(0xFF) != 0xFF);

    char acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (spi_transfer(0xFF) != 0xFF);

    if (earth->platform == ARTY) spi_set_clock(20000000);
    return 0;
}
