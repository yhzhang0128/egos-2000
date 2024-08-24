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

void disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == SD_CARD) {
        for (uint i = 0; i < nblocks; i++)
            sd_read(block_no + i, dst + BLOCK_SIZE * i);
    } else {
        char* src = (char*)BOARD_FLASH_ROM + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    }
}

void disk_write(uint block_no, uint nblocks, char* src) {
    if (type == FLASH_ROM) FATAL("disk_write: Writing to ROM");
    for (uint i = 0; i < nblocks; i++)
        sd_write(block_no + i, src + BLOCK_SIZE * i);
}

void disk_init() {
    earth->disk_read  = disk_read;
    earth->disk_write = disk_write;

    type = (sd_init() == 0) ? SD_CARD : FLASH_ROM;
    if (type == FLASH_ROM) CRITICAL("Failed at initializing SD card; Use FLASH ROM instead");
}

/* Two helper functions for the SPI bus */
#define LITEX_SPI_CONTROL    0UL
#define LITEX_SPI_STATUS     4UL
#define LITEX_SPI_MOSI       8UL
#define LITEX_SPI_MISO      12UL
#define LITEX_SPI_CS        16UL
#define LITEX_SPI_CLKDIV    24UL

#define SIFIVE_SPI_CSDEF    20UL
#define SIFIVE_SPI_CSMODE   24UL
#define SIFIVE_SPI_TXDATA   72UL
#define SIFIVE_SPI_RXDATA   76UL

char spi_transfer(char byte) {
    /* "transfer" means sending a byte and then receiving a byte */
    uint rxdata;
    if (earth->platform == ARTY) {
        REGW(SPI_BASE, LITEX_SPI_MOSI)    = byte;
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

/* Send SD card commands through the SPI bus */
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

/* Read/write SD card blocks */
static void sd_read(uint offset, char* dst) {
    /* QEMU uses the SD2 standard (offset is byte offset)
     * Arty uses the SDHC/SDXC standard (offset is block offset) */
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
    /* QEMU uses the SD2 standard (offset is byte offset)
     * Arty uses the SDHC/SDXC standard (offset is block offset) */
    if (earth->platform == QEMU) offset *= BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_transfer(0xFF) != 0xFF);

    /* Send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* Transfer 1-byte buffer before writing block */
    spi_transfer(0xFF);

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

/* Initialize the SD card during bootup */
static int sd_init() {
    /* Configure the SPI controller */
    INFO("Set the CS pin to HIGH and toggle clock");

    if (earth->platform == ARTY) {
        spi_set_clock(400000);
        REGW(SPI_BASE, LITEX_SPI_CS) = 0;
        for (uint i = 0; i < 1000; i++) spi_transfer(0xFF);
        REGW(SPI_BASE, LITEX_SPI_CS) = 1;
    } else {
        REGW(SPI_BASE, SIFIVE_SPI_CSMODE) = 1;
        for (uint i = 0; i < 1000; i++) spi_transfer(0xFF);
        REGW(SPI_BASE, SIFIVE_SPI_CSDEF) = 1;
    }

    INFO("Set the CS pin to LOW and send cmd0 to SD card");
    char reply, cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    if ((reply = sd_exec_cmd(cmd0)) == 0xFF) return -1;
    while (reply != 0x01) reply = spi_transfer(0xFF);
    while (spi_transfer(0xFF) != 0xFF);

    INFO("Check SD card type and voltage with cmd8");
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    reply = sd_exec_cmd(cmd8);

    if (reply & 0x04) {
        /* Illegal command */
        FATAL("Only SD2/SDHC/SDXC cards are supported");
    } else {
        /* Only need the last byte of the r7 response */
        uint payload;
        for (uint i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = spi_transfer(0xFF);
        INFO("SD card replies cmd8 with status %d", reply);

        if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");
    }
    while (spi_transfer(0xFF) != 0xFF);

    char acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (spi_transfer(0xFF) != 0xFF);

    if (earth->platform == ARTY) {
        INFO("Set the SPI clock to 20MHz for the SD card");
        spi_set_clock(20000000);
    }
    return 0;
}
