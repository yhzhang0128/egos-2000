/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Read and write data blocks of an SD card.
 */

#include "sd.h"
#include "disk.h"

static void single_read(int offset, char* dst) {
    /* wait until SD card is not busy */
    while (recv_data_byte() != 0xFF);
    
    /* send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(cmd17))
        FATAL("SD card replies cmd17 with status 0x%.2x", reply);
 
    /* wait for the data packet and ignore the 2-byte checksum */
    while (recv_data_byte() != 0xFE);
    for (int i = 0; i < BLOCK_SIZE; i++)
        dst[i] = recv_data_byte();
    recv_data_byte();
    recv_data_byte();
}

static void single_write(int offset, char* src) {
    /* wait until SD card is not busy */
    while (recv_data_byte() != 0xFF);

    /* send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* send data packet: token + block + dummy 2-byte checksum */
    send_data_byte(0xFE);
    for (int i = 0; i < BLOCK_SIZE; i++)
        send_data_byte(src[i]);
    send_data_byte(0xFF);
    send_data_byte(0xFF);

    /* wait for SD card ack of data packet */
    while ((reply = recv_data_byte()) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card write ack with status 0x%.2x", reply);
}

int sdread(int offset, int nblock, char* dst) {
    for (int i = 0; i < nblock; i++)
        single_read(offset + i, dst + BLOCK_SIZE * i);
    return 0;
}

int sdwrite(int offset, int nblock, char* src) {
    for (int i = 0; i < nblock; i++)
        single_write(offset + i, src + BLOCK_SIZE * i);
    return 0;
}
