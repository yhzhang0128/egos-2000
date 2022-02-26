/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: util functions for sending/receiving bytes to the SD card
 */

#include <stdio.h>
#include "sdutil.h"

char send_data_byte(struct metal_spi *spi, char byte) {
    long rxdata, control_base = SPI_BASE_ADDR;

    while (METAL_SPI_REGW(METAL_SIFIVE_SPI0_TXDATA) & METAL_SPI_TXDATA_FULL);
    METAL_SPI_REGB(METAL_SIFIVE_SPI0_TXDATA) = byte;

    while ((rxdata = METAL_SPI_REGW(METAL_SIFIVE_SPI0_RXDATA)) &
           METAL_SPI_RXDATA_EMPTY);    
    return (char)(rxdata & METAL_SPI_TXRXDATA_MASK);
}

inline char recv_data_byte(struct metal_spi *spi) {
    return send_data_byte(spi, 0xFF);
}

char sd_exec_cmd(struct metal_spi *spi, char* cmd) {
    int i;
    for (i = 0; i < 6; i++)
        send_data_byte(spi, cmd[i]);

    char reply;
    while (1) {
        if (i++ % 1000 == 0)
            printf("    ... wait for the SD card to reply cmd%d\r\n", cmd[0] ^ 0x40);

        if ((reply = recv_data_byte(spi)) != 0xFF)
            break;
    }

    return reply;    
}

char sd_exec_acmd(struct metal_spi *spi, char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (recv_data_byte(spi) != 0xFF);
    sd_exec_cmd(spi, cmd55);

    while (recv_data_byte(spi) != 0xFF);
    return sd_exec_cmd(spi, cmd);
}
