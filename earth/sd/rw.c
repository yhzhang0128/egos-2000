/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: read and write blocks of an SD card
 */


#include <stdio.h>
#include "sd.h"

static int single_read(uint32_t, char*);
static int single_write(uint32_t, char*);
static int multi_read(uint32_t, uint32_t, char*);
static int multi_write(uint32_t, uint32_t, char*);

int sdread(uint32_t offset, uint32_t nblock, char* dst) {
    for (int i = 0; i < nblock; i++)
        if (single_read(offset + i, dst + SD_BLOCK_SIZE * i))
            return -1;
    return 0;
}

int sdwrite(uint32_t offset, uint32_t nblock, char* src) {
    for (int i = 0; i < nblock; i++)
        if (single_write(offset + i, src + SD_BLOCK_SIZE * i))
            return -1;
    return 0;
}

static int single_read(uint32_t offset, char* dst) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte() != 0xFF);
    
    /* send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(cmd17))
        FATAL("[ERROR] SD card replies cmd17 with status 0x%.2x", reply);
 
    /* wait for the data packet and ignore the 2-byte checksum */
    char token;
    while ((token = recv_data_byte()) != 0xFE);
    for (int i = 0; i < SD_BLOCK_SIZE; i++)
        dst[i] = recv_data_byte();
    recv_data_byte();
    recv_data_byte();

    return 0;
}


static int single_write(uint32_t offset, char* src) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte() != 0xFF);

    /* send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* send data packet: token + block + dummy 2-byte checksum */
    send_data_byte(0xFE);
    for (int i = 0; i < SD_BLOCK_SIZE; i++)
        send_data_byte(src[i]);
    send_data_byte(0xFF);
    send_data_byte(0xFF);

    /* wait for SD card ack of data packet */
    while ((reply = recv_data_byte()) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card replies write data packet with status 0x%.2x", reply);
    
    return 0;
}
