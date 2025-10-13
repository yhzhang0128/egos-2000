/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include <string.h>

#define SDHCI_PCI_ECAM_BASE   0x30008000
#define SDHCI_BASE            0x40000000
#define SDHCI_SOFTWARE_RESET  0x2F
#define SDHCI_CAPABILITIES    0x40
#define SDHCI_INT_STAT_ENABLE 0x34
#define SDHCI_INT_SIG_ENABLE  0x38

static __attribute__((aligned(BLOCK_SIZE))) char aligned_buf[BLOCK_SIZE];

static void sd_read(uint offset, char* dst) { FATAL("sd_read end."); }

static int sd_init() {
    /* Set the PCI ECAM base address register as SDHCI_BASE */
    REGW(SDHCI_PCI_ECAM_BASE, 0x4)  = 0x2;
    REGW(SDHCI_PCI_ECAM_BASE, 0x10) = SDHCI_BASE;

    /* Reset the SD card */
    REGB(SDHCI_BASE, SDHCI_SOFTWARE_RESET) = 0x1;
    while (REGB(SDHCI_BASE, SDHCI_SOFTWARE_RESET) & 0x1);

    /* Enable only interrupts served by the SD controller */
    REGW(SDHCI_BASE, SDHCI_INT_STAT_ENABLE) = 0x27F003B;
    /* Mask all SDHCI interrupt sources */
    REGW(SDHCI_BASE, SDHCI_INT_SIG_ENABLE) = 0x0;

    return 0;
}

static enum disk_type { SD_CARD, FLASH_ROM } type;

void disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == FLASH_ROM) {
        char* src = (char*)BOARD_FLASH_ROM + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
        return;
    }

    /* Student's code goes here (Serial Device Driver). */

    /* Replace the loop below by reading multiple SD card
     * blocks altogether using the cmd18 SD card command. */
    for (uint i = 0; i < nblocks; i++)
        sd_read(block_no + i, dst + BLOCK_SIZE * i);

    /* Student's code ends here. */
}

void disk_write(uint block_no, uint nblocks, char* src) {
    FATAL("disk_write is not implemented");
}

void disk_init() {
    earth->disk_read  = disk_read;
    earth->disk_write = disk_write;

    type = (sd_init() == 0) ? SD_CARD : FLASH_ROM;
    if (type == FLASH_ROM) CRITICAL("Using FLASH_ROM instead of SD_CARD");
}
