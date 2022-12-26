/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: helper functions for communication with the SD card
 */

#include "sd.h"

char send_data_byte(char byte) {
    while (REGW(SPI1_BASE, SPI1_TXDATA) & (1 << 31));
    REGB(SPI1_BASE, SPI1_TXDATA) = byte;

    long rxdata;
    while ((rxdata = REGW(SPI1_BASE, SPI1_RXDATA)) & (1 << 31));
    return (char)(rxdata & 0xFF);
}

char recv_data_byte() { return send_data_byte(0xFF); }

char sd_exec_cmd(char* cmd) {
    for (int i = 0; i < 6; i++) send_data_byte(cmd[i]);

    for (int reply, i = 0; i < 8000; i++)
        if ((reply = recv_data_byte()) != 0xFF) return reply;

    FATAL("SD card not responding cmd%d", cmd[0] ^ 0x40);
}

char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (recv_data_byte() != 0xFF);
    sd_exec_cmd(cmd55);

    while (recv_data_byte() != 0xFF);
    return sd_exec_cmd(cmd);
}
