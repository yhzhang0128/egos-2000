/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: reset and initialize the SD card on Arty; this library works with SDHC, but may contain bugs for other types of SD cards
 */


#include <stdio.h>
#include "sd.h"

static int sd_spi_reset(struct metal_spi*);
static int sd_spi_configure(struct metal_spi*);

static void sd_print_type();
static int sd_check_type(struct metal_spi*);
static int sd_check_capacity(struct metal_spi*);

int SD_CARD_TYPE = SD_CARD_TYPE_UNKNOWN;

int sdinit() {
    long baud_rate = 100000;
    struct metal_spi *spi = metal_spi_get_device(0);
    metal_spi_set_baud_rate(spi, baud_rate);
    printf("[INFO] Set SPI clock frequency to %ldHz\r\n", baud_rate);

    if (0 != sd_spi_configure(spi)) {
        printf("[ERROR] Fail to configure spi device\r\n");
        return -1;
    }

    if (0 != sd_spi_reset(spi)) {
        printf("[ERROR] Fail to reset SD card with cmd0\r\n");
        return -1;        
    }

    long cpu_clock_rate = 65000000;
    baud_rate = cpu_clock_rate / 4;
    metal_spi_set_baud_rate(spi, baud_rate);
    printf("[INFO] Set SPI clock frequency to %ldHz\r\n", baud_rate);

    printf("[INFO] Check SD card type and voltage with cmd8\r\n");
    if (sd_check_type(spi) == -1) {
        printf("[ERROR] Fail to check SD card type and voltage\r\n");
        return -1;
    }

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_CARD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(spi, acmd41));
    for (int i = 0; i < 100; i++)
        recv_data_byte(spi);
    printf("[INFO] SD card initialization completes\r\n");

    if (SD_BLOCK_SIZE != 512) {
        printf("[ERROR] SD_BLOCK_SIZE is not 512");
        return -1;
    }
    printf("[INFO] Set block size to 512 bytes with cmd16\r\n");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    char reply = sd_exec_cmd(spi, cmd16);
    for (int i = 0; i < 100; i++)
        recv_data_byte(spi);
    printf("[INFO] SD card replies cmd16 with status 0x%.2x\r\n", reply);

    if (SD_CARD_TYPE == SD_CARD_TYPE_SD2 &&
        0 != sd_check_capacity(spi)) {
        return -1;
    }
    sd_print_type();

    return 0;
}

static int sd_check_type(struct metal_spi *spi) {
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    char reply = sd_exec_cmd(spi, cmd8);

    printf("[INFO] SD card replies cmd8 with status 0x%.2x\r\n", reply);
    if (reply & 0x04) {
        /* Illegal command */
        SD_CARD_TYPE = SD_CARD_TYPE_SD1;
    } else {
        // only need last byte of r7 response
        unsigned long payload;
        for (int i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = recv_data_byte(spi);
        printf("[INFO] SD card replies cmd8 with payload 0x%.8x\r\n", payload);

        if ((payload & 0xFFF) != 0x1AA)
            return -1;

        SD_CARD_TYPE = SD_CARD_TYPE_SD2;
    }

    for (int i = 0; i < 10; i++)
        recv_data_byte(spi);

    return 0;
}

static int sd_check_capacity(struct metal_spi *spi) {
    printf("[INFO] Check SD card capacity with cmd58\r\n");
    for (int i = 0; i < 10; i++)
        recv_data_byte(spi);

    char reply, cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
    if (sd_exec_cmd(spi, cmd58)) {
        printf("[ERROR] cmd58 returns failure\r\n");
        return -1;
    }
    printf("[INFO] SD card replies cmd58 with status 0x00\r\n");

    unsigned long payload;
    for (uint8_t i = 0; i < 4; i++) {
        reply = ((char*)&payload)[3 - i] = recv_data_byte(spi);

        if (i == 0 && ((reply & 0XC0) == 0XC0))
            SD_CARD_TYPE = SD_CARD_TYPE_SDHC;
    }
    printf("[INFO] SD card replies cmd58 with payload 0x%.8x\r\n", payload);

    for (int i = 0; i < 100; i++)
        recv_data_byte(spi);

    return 0;
}

static void sd_print_type() {
    switch (SD_CARD_TYPE) {
    case SD_CARD_TYPE_SDHC:
        printf("[INFO] SD card is high capacity SDHC card\r\n");
        break;
    default:
        printf("[FATAL] Unknown SD card type\r\n");
        while(1);
    }
}

static int sd_spi_reset(struct metal_spi *spi) {
    printf("[INFO] Set CS and MOSI to 1 and toggle clock.\r\n");
    long control_base = SPI_BASE_ADDR;
    printf("[INFO] UART base address is 0x%x.\r\n", control_base);
    /* Keep chip select line high */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) &= ~(METAL_SPI_CSMODE_MASK);
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) |= METAL_SPI_CSMODE_HOLD;

    unsigned long i, rxdata;
    for (i = 0; i < 100; i++) {
        if (i % 20 == 0)
            printf("    ... completed %d%c\r\n", i / 10 * 10, '%');

        send_data_byte(spi, 0xFF);
    }

    /* Keep chip select line low */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSDEF) = 1;
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) &= ~(METAL_SPI_CSMODE_MASK);
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) |= METAL_SPI_CSMODE_HOLD;
    for (i = 0; i < 200000; i++);
    
    printf("[INFO] Set CS to 0 and send cmd0 through MOSI.\r\n");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(spi, cmd0);

    while (reply != 0x01) {
        reply = recv_data_byte(spi);
    }
    printf("[INFO] SD card replies cmd0 with 0x01\r\n");

    for(i = 0; i < 10; i++)
        recv_data_byte(spi);
    return 0;
}

static int sd_spi_configure(struct metal_spi *spi) {
    long control_base = SPI_BASE_ADDR;

    /* Set protocol as METAL_SPI_SINGLE */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) &= ~(METAL_SPI_PROTO_MASK);
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) |= METAL_SPI_PROTO_SINGLE;

    /* Set polarity as 0 */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_SCKMODE) &=
        ~(1 << METAL_SPI_SCKMODE_POL_SHIFT);

    /* Set phase as 0*/
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_SCKMODE) &=
        ~(1 << METAL_SPI_SCKMODE_PHA_SHIFT);

    /* Set endianness as 0 */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) &= ~(METAL_SPI_ENDIAN_LSB);

    /* Always populate receive FIFO */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) &= ~(METAL_SPI_DISABLE_RX);

    /* Set CS active-high as 0 */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSDEF) = 0;

    /* Set frame length */
    if ((METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) & METAL_SPI_FRAME_LEN_MASK) !=
        (8 << METAL_SPI_FRAME_LEN_SHIFT)) {
        METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) &= ~(METAL_SPI_FRAME_LEN_MASK);
        METAL_SPI_REGW(METAL_SIFIVE_SPI0_FMT) |=
            (8 << METAL_SPI_FRAME_LEN_SHIFT);
    }

    /* Set CS line */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSID) = 0;

    /* Toggle off memory-mapped SPI flash mode
     * toggle on programmable IO mode */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_FCTRL) = METAL_SPI_CONTROL_IO;

    return 0;
}
