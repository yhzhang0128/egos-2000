/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: util functions for sending/receiving bytes to the SD card
 */

#include <stdio.h>
#include "sd.h"

char send_data_byte(char byte) {
    long rxdata, control_base = SPI_BASE_ADDR;

    while (METAL_SPI_REGW(METAL_SIFIVE_SPI0_TXDATA) & METAL_SPI_TXDATA_FULL);
    METAL_SPI_REGB(METAL_SIFIVE_SPI0_TXDATA) = byte;

    while ((rxdata = METAL_SPI_REGW(METAL_SIFIVE_SPI0_RXDATA)) &
           METAL_SPI_RXDATA_EMPTY);    
    return (char)(rxdata & METAL_SPI_TXRXDATA_MASK);
}

inline char recv_data_byte() {
    return send_data_byte(0xFF);
}

char sd_exec_cmd(char* cmd) {
    for (int i = 0; i < 6; i++)
        send_data_byte(cmd[i]);

    for (int reply, i = 0; i < 5000; i++) {
        if (i % 1000 == 0)
            INFO("    ... wait for the SD card to reply cmd%d", cmd[0] ^ 0x40);
        if ((reply = recv_data_byte()) != 0xFF) return reply;
    }

    FATAL("SD card not responding cmd%d", cmd[0] ^ 0x40);
}

char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (recv_data_byte() != 0xFF);
    sd_exec_cmd(cmd55);

    while (recv_data_byte() != 0xFF);
    return sd_exec_cmd(cmd);
}
