/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"
#include "sd/sd.h"
#include <string.h>

enum disk_type {
      SD_CARD,
      FLASH_ROM
};
static enum disk_type type;

int disk_read(uint block_no, uint nblocks, char* dst) {
    if (type == SD_CARD) {
        sdread(block_no, nblocks, dst);
    } else {
        char* src = (char*)0x20800000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    }
    return 0;
}

int disk_write(uint block_no, uint nblocks, char* src) {
    if (type == FLASH_ROM)
        FATAL("disk_write: Writing to the read-only ROM");

    sdwrite(block_no, nblocks, src);
    return 0;
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

    if (earth->platform == ARTY) {
        type = FLASH_ROM;
    } else {
        type = SD_CARD;
        sdinit();
    }
}
