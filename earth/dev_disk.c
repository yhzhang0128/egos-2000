/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include "sd/sd.h"
#include "bus_gpio.c"
#include <string.h>

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
    if (type == FLASH_ROM) FATAL("Attempt to write the on-board ROM");
    sdwrite(block_no, nblocks, src);
    return 0;
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

    if (earth->platform == QEMU) {
        /* QEMU only uses the on-board ROM as disk;
         * SD card is only supported on the Arty board */
        type = FLASH_ROM;
        return;
    }

    CRITICAL("Choose a disk:");
    printf("Enter 0: microSD card\r\nEnter 1: on-board ROM\r\n");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(buf, 2));
    type = (buf[0] == '0')? SD_CARD : FLASH_ROM;
    INFO("%s is chosen", type == SD_CARD? "microSD" : "on-board ROM");

    if (type == SD_CARD) sdinit();
}
