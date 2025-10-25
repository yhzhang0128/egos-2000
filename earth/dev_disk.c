/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include <string.h>

#define SDHCI_DMA_ADDRESS      0x00
#define SDHCI_BLK_CNT_AND_SIZE 0x04
#define SDHCI_ARGUMENT         0x08
#define SDHCI_CMD_AND_MODE     0x0C
#define SDHCI_RESPONSE0        0x10
#define SDHCI_PRESENT_STATE    0x24
#define SDHCI_CLKCON           0x2C
#define SDHCI_SOFTWARE_RESET   0x2F
#define SDHCI_INT_STAT         0x30
#define SDHCI_INT_STAT_ENABLE  0x34
#define SDHCI_INT_SIG_ENABLE   0x38

static char sdhci_exec_cmd(uint idx, uint arg, uchar flag, uint mode) {
    /* Wait until the SD controller to be ready for a new command. */
    while (REGW(SDHCI_BASE, SDHCI_PRESENT_STATE) & 0x3);

    /* Clear the interrupt status register. */
    REGW(SDHCI_BASE, SDHCI_INT_STAT) = 0xFFFFFFFF;

    /* Issue the command. */
    REGW(SDHCI_BASE, SDHCI_ARGUMENT)     = arg;
    REGW(SDHCI_BASE, SDHCI_CMD_AND_MODE) = (((idx << 8) | flag) << 16) | mode;

    /* Wait for the command to be completed. */
    while (!(REGW(SDHCI_BASE, SDHCI_INT_STAT) & 0x1));
}

static void sdhci_read(uint offset, char* dst) {
    /* Prepare DMA (SDMA mode of SDHCI). */
    static __attribute__((aligned(BLOCK_SIZE))) char aligned_buf[BLOCK_SIZE];
    REGW(SDHCI_BASE, SDHCI_DMA_ADDRESS)      = (uint)aligned_buf;
    REGW(SDHCI_BASE, SDHCI_BLK_CNT_AND_SIZE) = (1 << 16) | BLOCK_SIZE;

#define DATA_PRESENT_FLAG         (1 << 5)
#define READ_WITH_DMA_ENABLE_MODE ((1 << 4) | (1 << 0))
    /* Send and wait for a read request with command #17. */
    offset *= BLOCK_SIZE;
    sdhci_exec_cmd(17, offset, DATA_PRESENT_FLAG, READ_WITH_DMA_ENABLE_MODE);

    memcpy(dst, aligned_buf, BLOCK_SIZE);
}

static int sdhci_init() {
#define PCI_ECAM_ALLOW_MMIO_AND_DMA ((1 << 1) | (1 << 2))
    /* Set the PCI ECAM base address register as SDHCI_BASE. */
    REGW(SDHCI_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;
    REGW(SDHCI_PCI_ECAM, 0x10) = SDHCI_BASE;

    /* Reset the SD card and enable clock. */
    REGB(SDHCI_BASE, SDHCI_SOFTWARE_RESET) = 0x1;
    while (REGB(SDHCI_BASE, SDHCI_SOFTWARE_RESET) & 0x1);
    REGB(SDHCI_BASE, SDHCI_CLKCON) = 0x5;

    /* Enable interrupt status, but disable interrupt signal. */
    REGW(SDHCI_BASE, SDHCI_INT_SIG_ENABLE)  = 0x0;
    REGW(SDHCI_BASE, SDHCI_INT_STAT_ENABLE) = 0x27F003B;

    /* A simplified SDHCI initialization tailored for QEMU. */
    sdhci_exec_cmd(55, 0, 0, 0);
    sdhci_exec_cmd(41, 0xFFF0000, 0, 0);
    sdhci_exec_cmd(2, 0, 0, 0);
    sdhci_exec_cmd(3, 0, 2 /* get response */, 0);
    sdhci_exec_cmd(7, REGW(SDHCI_BASE, SDHCI_RESPONSE0), 0, 0);
}

#define LITEX_SPI_CONTROL 0UL
#define LITEX_SPI_STATUS  4UL
#define LITEX_SPI_MOSI    8UL
#define LITEX_SPI_MISO    12UL
#define LITEX_SPI_CS      16UL
#define LITEX_SPI_CLKDIV  24UL

static char spi_exchange(char byte) {
    /* The "exchange" here means sending a byte and then receiving a byte. */
    REGW(SDSPI_BASE, LITEX_SPI_MOSI)    = byte;
    REGW(SDSPI_BASE, LITEX_SPI_CONTROL) = (8 * (1 << 8) | (1));

    while ((REGW(SDSPI_BASE, LITEX_SPI_STATUS) & 1) != 1);
    return (char)(REGW(SDSPI_BASE, LITEX_SPI_MISO) & 0xFF);
}

static char sdspi_exec_cmd(char* cmd) {
    /* Send a 6-byte SD card command through the SPI bus. */
    for (uint i = 0; i < 6; i++) spi_exchange(cmd[i]);
    for (uint reply, i = 0; i < 8000; i++)
        if ((reply = spi_exchange(0xFF)) != 0xFF) return reply;

    return 0xFF;
}

static char sdspi_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (spi_exchange(0xFF) != 0xFF);
    sdspi_exec_cmd(cmd55);

    while (spi_exchange(0xFF) != 0xFF);
    return sdspi_exec_cmd(cmd);
}

static void sdspi_read(uint offset, char* dst) {
    /* Wait until SD card is ready for a new command. */
    while (spi_exchange(0xFF) != 0xFF);

    /* Send a read request with command #17. */
    char* arg = (void*)&offset;
    char reply, cmd17[] = {17 | (1 << 6), arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sdspi_exec_cmd(cmd17))
        FATAL("cmd17 returns status 0x%.2x", reply);

    /* Wait for the data packet and ignore the 2-byte checksum. */
    while (spi_exchange(0xFF) != 0xFE);
    for (uint i = 0; i < BLOCK_SIZE; i++) dst[i] = spi_exchange(0xFF);
    spi_exchange(0xFF);
    spi_exchange(0xFF);
}

static int sdspi_init() {
    /* Configure the SPI controller. */
#define CPU_CLOCK_RATE 100000000 /* 100MHz */
    INFO("Set the CS pin to HIGH and toggle clock");
    REGW(SDSPI_BASE, LITEX_SPI_CLKDIV) = CPU_CLOCK_RATE / 400000 + 1;
    REGW(SDSPI_BASE, LITEX_SPI_CS)     = 0;
    for (uint i = 0; i < 1000; i++) spi_exchange(0xFF);
    REGW(SDSPI_BASE, LITEX_SPI_CS) = 1;

    INFO("Set the CS pin to LOW and send cmd0 to SD card");
    char reply, cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    if ((reply = sdspi_exec_cmd(cmd0)) == 0xFF) return -1;
    while (reply != 0x01) reply = spi_exchange(0xFF);
    while (spi_exchange(0xFF) != 0xFF);

    INFO("Check SD card type and voltage with cmd8");
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    reply       = sdspi_exec_cmd(cmd8);
    INFO("SD card replies cmd8 with status %d", reply);
    if (reply & 0x04) FATAL("Only SD2/SDHC/SDXC cards are supported");

    /* We only need the last byte of the r7 response. */
    uint payload;
    for (uint i = 0; i < 4; i++) ((char*)&payload)[3 - i] = spi_exchange(0xFF);
    if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");

    while (spi_exchange(0xFF) != 0xFF);
    char acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF};
    while (sdspi_exec_acmd(acmd41));
    while (spi_exchange(0xFF) != 0xFF);

    INFO("Set the SPI clock to 20MHz for the SD card");
    REGW(SDSPI_BASE, LITEX_SPI_CLKDIV) = CPU_CLOCK_RATE / 20000000 + 1;

    return 0;
}

static enum disk_type { SD_CARD, FLASH_ROM } type;

void disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == FLASH_ROM) {
        char* src = (char*)FLASH_ROM_BASE + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
        return;
    }

    /* Student's code goes here (Serial Device Driver). */

    /* Replace the loop below by reading multiple SD card
     * blocks altogether using the SD card command #18. */
    for (uint i = 0; i < nblocks; i++)
        (earth->platform == HARDWARE)
            ? sdspi_read(block_no + i, dst + BLOCK_SIZE * i)
            : sdhci_read(block_no + i, dst + BLOCK_SIZE * i);

    /* Student's code ends here. */
}

void disk_write(uint block_no, uint nblocks, char* src) {
    if (type == FLASH_ROM) FATAL("FLASH_ROM is read only");
    /* Student's code goes here (Serial Device Driver). */

    /* Implement SD card write using SPI or SDHCI+PCI. */

    /* Student's code ends here. */
}

void disk_init() {
    earth->disk_read  = disk_read;
    earth->disk_write = disk_write;

    if (earth->platform == QEMU) {
        /* QEMU uses the PCI bus and the SDHCI standard. */
        sdhci_init();
    } else {
        /* Hardware uses the SPI bus to control SD card. */
        type = (sdspi_init() == 0) ? SD_CARD : FLASH_ROM;
        if (type == FLASH_ROM) CRITICAL("Using FLASH_ROM instead of SD_CARD");
    }
}
