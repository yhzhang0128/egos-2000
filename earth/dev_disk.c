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

int disk_read(int block_no, int nblocks, char* dst) {
    /* Flash the led0 blue light */
    *(char*)(0x1001200C) |= 8;
    
    if (type == SD_CARD) {
        return sdread(block_no, nblocks, dst);
    } else {
        char* src = (char*)0x20800000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
        return 0;
    }
}

int disk_write(int block_no, int nblocks, char* src) {
    /* Flash the led0 red light */
    *(char*)(0x1001200C) |= 1;

    if (type == SD_CARD)
        return sdwrite(block_no, nblocks, src);
    else
        FATAL("Try to write the on-board flash ROM"); 
}

void disk_init() {
    HIGHLIGHT("Choose a disk:");
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
