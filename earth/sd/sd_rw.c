/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: read and write SD card blocks
 */

#include "sd.h"
#include "disk.h"

static void single_read(int offset, char* dst) {
    /* Wait until SD card is not busy */
    while (recv_data_byte() != 0xFF);
    
    /* Send read request with cmd17 */
    char *arg = (void*)&offset;
    char reply, cmd17[] = {0x51, arg[3], arg[2], arg[1], arg[0], 0xFF};

    if (reply = sd_exec_cmd(cmd17))
        FATAL("SD card replies cmd17 with status 0x%.2x", reply);
 
    /* Wait for the data packet and ignore the 2-byte checksum */
    while (recv_data_byte() != 0xFE);
    for (int i = 0; i < BLOCK_SIZE; i++) dst[i] = recv_data_byte();
    recv_data_byte();
    recv_data_byte();
}

static void single_write(int offset, char* src) {
    /* Wait until SD card is not busy */
    while (recv_data_byte() != 0xFF);

    /* Send write request with cmd24 */
    char *arg = (void*)&offset;
    char reply, cmd24[] = {0x58, arg[3], arg[2], arg[1], arg[0], 0xFF};
    if (reply = sd_exec_cmd(cmd24))
        FATAL("SD card replies cmd24 with status %.2x", reply);

    /* Send data packet: token + block + dummy 2-byte checksum */
    send_data_byte(0xFE);
    for (int i = 0; i < BLOCK_SIZE; i++) send_data_byte(src[i]);
    send_data_byte(0xFF);
    send_data_byte(0xFF);

    /* Wait for SD card ack of data packet */
    while ((reply = recv_data_byte()) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card write ack with status 0x%.2x", reply);
}

int sdread(int offset, int nblock, char* dst) {
    /* A better way to read multiple blocks using SD card
     * command 18 is left to students as a course project */

    for (int i = 0; i < nblock; i++)
        single_read(offset + i, dst + BLOCK_SIZE * i);
    return 0;
}

int sdwrite(int offset, int nblock, char* src) {
    /* A better way to write multiple blocks using SD card
     * command 25 is left to students as a course project */

    for (int i = 0; i < nblock; i++)
        single_write(offset + i, src + BLOCK_SIZE * i);
    return 0;
}
