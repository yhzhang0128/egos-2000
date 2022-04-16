/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple disk device driver
 */


#include "earth.h"
#include "sd/sd.h"
#include "bus_gpio.c"

enum {
      SD_CARD,
      FLASH_ROM
};
static int type;

int disk_read(int block_no, int nblocks, char* dst) {
    if (type == SD_CARD) {
        sdread(block_no, nblocks, dst);
    } else {
        char* src = (char*)0x20800000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    }
    return 0;
}

int disk_write(int block_no, int nblocks, char* src) {
    if (type == FLASH_ROM) FATAL("disk_write: try to write the flash ROM");
    sdwrite(block_no, nblocks, src);
    return 0;
}

void disk_init() {
    CRITICAL("Choose a disk:");
    printf("  Enter 0: use the microSD card\r\n");
    printf("  Enter 1: use the on-board flash ROM @0x20800000\r\n");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; tty_read(buf, 2));

    if (buf[0] == '0') {
        type = SD_CARD;
        INFO("microSD card is chosen");
        sdinit();
    } else {
        type = FLASH_ROM;
        INFO("on-board flash ROM is chosen");
    }
}
