/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: read and write blocks of an SD card
 */


#include <stdio.h>
#include "sdlib.h"
#include "sdutil.h"

static int single_read(uint32_t, char*);
static int single_write(uint32_t, char*);
static int multi_read(uint32_t, uint32_t, char*);
static int multi_write(uint32_t, uint32_t, char*);

int sdread(uint32_t offset, uint32_t nblock, char* dst) {
    if (nblock == 0)
        return 0;

    if (nblock == 1)
        return single_read(offset, dst);

    return multi_read(offset, nblock, dst);
}

int sdwrite(uint32_t offset, uint32_t nblock, char* src) {
    if (nblock == 0)
        return 0;

    if (nblock == 1)
        return single_write(offset, src);

    return multi_write(offset, nblock, src);
}    


static int single_read(uint32_t offset, char* dst) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte(spi) != 0xFF);
    
    /* send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(spi, cmd17)) {
        printf("[ERROR] SD card replies cmd17 with status 0x%.2x\r\n", reply);
        return -1;
    }
 
    /* wait for the data packet and ignore checksum */
    char token;
    while ((token = recv_data_byte(spi)) != 0xFE);
    for (int i = 0; i < SD_BLOCK_SIZE; i++) {
        dst[i] = recv_data_byte(spi);
    }
    recv_data_byte(spi);
    recv_data_byte(spi);

    return 0;
}


static int single_write(uint32_t offset, char* src) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte(spi) != 0xFF);

    /* send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(spi, cmd24)) {
        printf("[ERROR] SD card replies cmd24 with status %.2x\r\n", reply);
        return -1;
    }

    /* send data packet: token + block + dummy checksum */
    send_data_byte(spi, 0xFE);
    for (int i = 0; i < SD_BLOCK_SIZE; i++)
        send_data_byte(spi, src[i]);
    send_data_byte(spi, 0xFF);
    send_data_byte(spi, 0xFF);

    /* wait for SD card ack of data packet */
    while ((reply = recv_data_byte(spi)) == 0xFF);
    if ((reply & 0x1F) != 0x05) {
        printf("[ERROR] SD card replies write data packet with status 0x%.2x\r\n", reply);
        return -1;
    }
    
    return 0;
}


static int multi_read(uint32_t offset, uint32_t nblock, char* dst) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte(spi) != 0xFF);
    
    /* send multi-read request with cmd18 */
    char *arg = (void*)&offset;
    char reply, cmd18[] = {0x52, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(spi, cmd18)) {
        printf("[ERROR] SD card replies cmd18 with status 0x%.2x\r\n", reply);
        return -1;
    }
 
    for (int b = 0, off = 0; b < nblock; b++, off += SD_BLOCK_SIZE) {
        /* wait for the data packet and ignore checksum */
        while (recv_data_byte(spi) != 0xFE);
        for (int i = 0; i < SD_BLOCK_SIZE; i++) {
            dst[off + i] = recv_data_byte(spi);
        }
        recv_data_byte(spi);
        recv_data_byte(spi);
    }

    /* stop multi-read with cmd12 */
    char cmd12[] = {0x4c, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (recv_data_byte(spi) != 0xFF);
    sd_exec_cmd(spi, cmd12);
    while (recv_data_byte(spi) != 0xFF);

    return 0;    
}


static int multi_write(uint32_t offset, uint32_t nblock, char* src) {
    /* old SD cards use byte offset instead of block offset */
    if (SD_CARD_TYPE != SD_CARD_TYPE_SDHC)
        offset <<= 9;

    /* wait until SD card is not busy */
    struct metal_spi *spi = metal_spi_get_device(0);
    while (recv_data_byte(spi) != 0xFF);

    /* send multi-write request with cmd25 */
    char *arg = (void*)&offset;
    char reply, cmd25[] = {0x59, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(spi, cmd25)) {
        printf("[ERROR] SD card replies cmd25 with status %.2x\r\n", reply);
        return -1;
    }

    recv_data_byte(spi);
    for (int b = 0, off = 0; b < nblock; b++, off += SD_BLOCK_SIZE) {
        /* send data packet: token + block + dummy checksum */
        send_data_byte(spi, 0xFC);
        for (int i = 0; i < SD_BLOCK_SIZE; i++)
            send_data_byte(spi, src[off + i]);
        send_data_byte(spi, 0xFF);
        send_data_byte(spi, 0xFF);

        /* wait for SD card ack of data packet */
        while ((reply = recv_data_byte(spi)) == 0xFF);
        if ((reply & 0x1F) != 0x05) {
            printf("[ERROR] SD card replies multi-write data packet with status 0x%.2x for block #%d\r\n", reply, b);
            return -1;
        }

        /* wait for SD card completes flash write */
        while (recv_data_byte(spi) != 0xFF);
    }

    /* 0xFD is the stop transition token */
    send_data_byte(spi, 0xFD);
    while (recv_data_byte(spi) != 0xFF);
 
    return 0;
}
