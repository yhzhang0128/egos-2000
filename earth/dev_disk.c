/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include <string.h>

/* See the "SD Host Controller Simplified Specification" (Part A2) document
   from the SD Association (https://www.sdcard.org/downloads/pls/) in which
   Chapter 2 "SD Host Standard Register" defines the register offsets below. */
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

static void sdhci_exec_stop_cmd() {
    while (REGW(SDHCI_BASE, SDHCI_PRESENT_STATE) & 0x1);
    REGW(SDHCI_BASE, SDHCI_INT_STAT) = 0xFFFFFFFF;
    REGW(SDHCI_BASE, SDHCI_ARGUMENT) = 0;
    REGW(SDHCI_BASE, SDHCI_CMD_AND_MODE) = (((12 << 8) | 3) << 16);
    while (!(REGW(SDHCI_BASE, SDHCI_INT_STAT) & 0x1));
}

static void sdhci_read(uint offset, char* dst) {
    /* Prepare DMA (SDMA mode of SDHCI). */
    static __attribute__((aligned(BLOCK_SIZE))) char aligned_buf[BLOCK_SIZE];
    REGW(SDHCI_BASE, SDHCI_DMA_ADDRESS)      = (uint)aligned_buf;
    REGW(SDHCI_BASE, SDHCI_BLK_CNT_AND_SIZE) = (1 << 16) | BLOCK_SIZE;
    // asks SDHCI to write 1 block of BLOCK_SIZE bytes to aligned_buf, when executing upcoming SD read command


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
    for (uint reply, i = 0; i < 8000; i++) // 8000 is TIMEOUT; waits for the reply to happen, then returns the reply; otherwise, returns 0xFF after timeout
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

/*
SD card command 17 is defined for reading a block
- SD card block is 512 bytes
- when read/writing an SD card, OS will do it in blocks of 512 bytes

offset is used to determine which block should be read
read a block from SD card to memory address specified by dst
*/
static void sdspi_read(uint offset, char* dst) {
    /* Wait until SD card is ready for a new command. */
    while (spi_exchange(0xFF) != 0xFF);

    /* Send a read request with command #17. */
    char* arg = (void*)&offset;
    // 4 bytes in the middle encode offset
    // 
    char reply, cmd17[] = {17 | (1 << 6), arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sdspi_exec_cmd(cmd17))
        FATAL("cmd17 returns status 0x%.2x", reply);

    /* Wait for the data packet and ignore the 2-byte checksum. */
    // receive 512 bytes from the SD, and use 2 byte checksum
    // checksum is just a quick way to ensure the data is correct
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

/*
SD card standard provides command 18/25 for read/writes consecutive blocks together
- replace loop in disk_read
- implement disk_write with own SD card driver using 18/25

*/

static void sdspi_read_multi(uint offset, uint nblocks, char* dst){
    // wait until exchange is ready
    while(spi_exchange(0xFF) != 0xFF);
    char* arg = (void*)&offset;
    char reply, cmd18[] = {18 | (1 << 6), arg[3], arg[2], arg[1], arg[0], 0xFF};

    // exec 18, then run until down, then 12 to end the data stream
    if ((reply = sdspi_exec_cmd(cmd18)))
        FATAL("cmd18 returns status 0x%.2x", reply);

    for (uint b = 0; b < nblocks; b++) {
        while (spi_exchange(0xFF) != 0xFE);

        for (uint i = 0; i < BLOCK_SIZE; i++)
            dst[b * BLOCK_SIZE + i] = spi_exchange(0xFF);

        spi_exchange(0xFF);
        spi_exchange(0xFF);
    }

    char cmd12[] = {12 | (1 << 6), 0, 0, 0, 0, 0xFF};

    for (uint i = 0; i < 6; i++) spi_exchange(cmd12[i]);
    spi_exchange(0xFF); /* stuff byte after CMD12, cant use sdspi_exec_cmd, bc of stuff byte; otherwise, format is same*/

    for (uint i = 0; i < 8000; i++)
        if ((reply = spi_exchange(0xFF)) != 0xFF) break;

    if (reply) FATAL("cmd12 returns status 0x%.2x", reply);

    while (spi_exchange(0xFF) != 0xFF);
}

static void sdhci_read_multi(uint offset, uint nblocks, char* dst) {
#define SDHCI_READ_MULTI_MAX_BLOCKS 128
    if (nblocks == 0) return;

    //starting address is forced to be BLOCK_SIZE aligned, so we can read multiple blocks together; otherwise, SDHCI will return an error
    static __attribute__((aligned(BLOCK_SIZE)))
    char aligned_buf[SDHCI_READ_MULTI_MAX_BLOCKS * BLOCK_SIZE];

#define DATA_PRESENT_FLAG               (1 << 5)
#define READ_MULTI_WITH_DMA_ENABLE_MODE ((1 << 5) | (1 << 4) | (1 << 1) | (1 << 0))

    if (nblocks > SDHCI_READ_MULTI_MAX_BLOCKS)
        FATAL("sdhci_read_multi: too many blocks %d", nblocks);

    REGW(SDHCI_BASE, SDHCI_DMA_ADDRESS) = (uint)aligned_buf;
    REGW(SDHCI_BASE, SDHCI_BLK_CNT_AND_SIZE) =
        (nblocks << 16) | BLOCK_SIZE;

    sdhci_exec_cmd(18, offset * BLOCK_SIZE,
                   DATA_PRESENT_FLAG,
                   READ_MULTI_WITH_DMA_ENABLE_MODE);

    while (!(REGW(SDHCI_BASE, SDHCI_INT_STAT) & (1 << 1)));
    sdhci_exec_stop_cmd();

    memcpy(dst, aligned_buf, nblocks * BLOCK_SIZE);
}

void disk_read(uint block_no, uint nblocks, char* dst) {
    if (nblocks == 0) return;

    if (type == FLASH_ROM) {
        char* src = (char*)FLASH_ROM_BASE + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
        return;
    }

    /* Student's code goes here (I/O Device Driver). */

    /* Replace the loop below by reading multiple SD card
     * blocks altogether using the SD card command #18. */
    (earth->platform == HARDWARE)
        ? sdspi_read_multi(block_no, nblocks, dst)
        : sdhci_read_multi(block_no, nblocks, dst);
    // read from block_no + i (which is the offset, within SD card) to dst + BLOCK_SIZE * i (which is the destination memory address for the block read from SD card)

    /* Student's code ends here. */
}

static void sdspi_write_multi(uint offset, uint nblocks, char* src) {
    if (nblocks == 0) return;

    while (spi_exchange(0xFF) != 0xFF);

    char* arg = (void*)&offset;
    char reply, cmd25[] = {25 | (1 << 6), arg[3], arg[2], arg[1], arg[0], 0xFF};
    if ((reply = sdspi_exec_cmd(cmd25)))
        FATAL("cmd25 returns status 0x%.2x", reply);

    for (uint b = 0; b < nblocks; b++) {
        spi_exchange(0xFC); /* Start multi-block write token. */

        for (uint i = 0; i < BLOCK_SIZE; i++)
            spi_exchange(src[b * BLOCK_SIZE + i]);

        spi_exchange(0xFF); /* Dummy CRC. */
        spi_exchange(0xFF);

        reply = spi_exchange(0xFF);
        if ((reply & 0x1F) != 0x05)
            FATAL("cmd25 data response 0x%.2x", reply);

        while (spi_exchange(0xFF) != 0xFF);
    }

    spi_exchange(0xFD); /* Stop transmission token for SPI multi-write. */
    while (spi_exchange(0xFF) != 0xFF);
}

static void sdhci_write_multi(uint offset, uint nblocks, char* src) {
#define SDHCI_WRITE_MULTI_MAX_BLOCKS 128
    if (nblocks == 0) return;

    static __attribute__((aligned(BLOCK_SIZE)))
    char aligned_buf[SDHCI_WRITE_MULTI_MAX_BLOCKS * BLOCK_SIZE];

#define WRITE_MULTI_WITH_DMA_ENABLE_MODE ((1 << 5) | (1 << 1) | (1 << 0))

    if (nblocks > SDHCI_WRITE_MULTI_MAX_BLOCKS)
        FATAL("sdhci_write_multi: too many blocks %d", nblocks);

    memcpy(aligned_buf, src, nblocks * BLOCK_SIZE);

    REGW(SDHCI_BASE, SDHCI_DMA_ADDRESS) = (uint)aligned_buf;
    REGW(SDHCI_BASE, SDHCI_BLK_CNT_AND_SIZE) =
        (nblocks << 16) | BLOCK_SIZE;

    sdhci_exec_cmd(25, offset * BLOCK_SIZE,
                   DATA_PRESENT_FLAG,
                   WRITE_MULTI_WITH_DMA_ENABLE_MODE);

    while (!(REGW(SDHCI_BASE, SDHCI_INT_STAT) & (1 << 1)));
    sdhci_exec_stop_cmd();
}

void disk_write(uint block_no, uint nblocks, char* src) {
    if (nblocks == 0) return;
    if (type == FLASH_ROM) FATAL("FLASH_ROM is read only");
    /* Student's code goes here (I/O Device Driver). */

    /* Implement SD card write using SPI or SDHCI+PCI. */
    (earth->platform == HARDWARE)
        ? sdspi_write_multi(block_no, nblocks, src)
        : sdhci_write_multi(block_no, nblocks, src);

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

/*
SPI is simple way to access SD card, but its slow
- SPI devices need manual setup, can't be automatically detected
- restricted to 512byte chunks at a time

Introduce PLUG AND PLAY and DIRECT MEMORY ACCESS

Plug and Play
- allow OS to detect a new device connected with the CPU, and configure memory mapped IO for the device

DMA 
- allows hardware to read/write directly to disk, without CPU having to run SPI

P&P and DMA are possible bc of Peripheral Component Interconnect (PCI or PCIe) bus, which is a standard for connecting peripheral devices to CPU

in EGOS, memory regions are allocated for PCIe

PCIe bus provides advanced memory mapped IO interface for SD cards called Secure Digital Host Controller Interface (SDHCI)
- 
*/