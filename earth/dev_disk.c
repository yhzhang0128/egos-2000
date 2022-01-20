/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple disk device driver
 */


#include "earth.h"
#include "sd/sd.h"

enum {
      SD_CARD,
      FLASH_ROM
};

static int type;
#define FLASH_ROM_START 0x20800000

int disk_read(int block_no, int nblocks, char* dst) {
    if (type == SD_CARD) {
        return sdread(block_no, nblocks, dst);
    } else {
        char* src = (void*)FLASH_ROM_START;
        src += block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
        return 0;
    }
}

int disk_write(int block_no, int nblocks, char* src) {
    if (type == SD_CARD) {
        return sdwrite(block_no, nblocks, src);
    } else {
        FATAL("on-board flash ROM cannot be written");
    }    
}

int disk_init() {
    HIGHLIGHT("Choose a disk:");
    tty_write("  Enter 0: use the microSD card\r\n");
    tty_write("  Enter 1: use the on-board flash ROM @0x%.8x\r\n", FLASH_ROM_START);

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; tty_read(buf, 2));

    if (buf[0] == '0') {
        type = SD_CARD;
        INFO("microSD card is chosen");
        return sdinit();
    } else {
        type = FLASH_ROM;
        INFO("on-board flash ROM is chosen");
        return 0;
    }
}
