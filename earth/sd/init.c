/*
 * (C) 2021, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: reset and initialize the SD card on Arty; this library works with SDHC, but may contain bugs for other types of SD cards
 */


#include <stdio.h>
#include "sd.h"

static int sd_spi_reset();
static int sd_spi_configure();
static void sd_spi_set_clock(long baud_rate);

static void sd_print_type();
static int sd_check_type();
static int sd_check_capacity();

int SD_CARD_TYPE = SD_CARD_TYPE_UNKNOWN;

int sdinit() {
    sd_spi_set_clock(100000);
    INFO("Set SPI clock frequency to 100000Hz");
    if (0 != sd_spi_configure()) FATAL("Fail to configure spi device");
    if (0 != sd_spi_reset()) FATAL("Fail to reset SD card with cmd0");

    sd_spi_set_clock(65000000 / 4);
    INFO("Set SPI clock frequency to %ldHz", 65000000 / 4);

    INFO("Check SD card type and voltage with cmd8");
    if (0 != sd_check_type()) FATAL("Fail to check SD card type and voltage");

    char acmd41[] = {0x69, (SD_CARD_TYPE == SD_CARD_TYPE_SD2)? 0x40 : 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    for (int i = 0; i < 100; i++)
        recv_data_byte();
    INFO("SD card initialization completes");

    INFO("Set block size to 512 bytes with cmd16");
    char cmd16[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xFF};
    char reply = sd_exec_cmd(cmd16);
    for (int i = 0; i < 100; i++)
        recv_data_byte();
    INFO("SD card replies cmd16 with status 0x%.2x", reply);

    if (SD_CARD_TYPE == SD_CARD_TYPE_SD2)
        sd_check_capacity();
    sd_print_type();

    return 0;
}

static int sd_check_type() {
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    char reply = sd_exec_cmd(cmd8);

    INFO("SD card replies cmd8 with status 0x%.2x", reply);
    if (reply & 0x04) {
        /* Illegal command */
        SD_CARD_TYPE = SD_CARD_TYPE_SD1;
    } else {
        /* only need last byte of r7 response */
        unsigned long payload;
        for (int i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = recv_data_byte();
        INFO("SD card replies cmd8 with payload 0x%.8x", payload);

        if ((payload & 0xFFF) != 0x1AA) return -1;
        SD_CARD_TYPE = SD_CARD_TYPE_SD2;
    }

    for (int i = 0; i < 10; i++)
        recv_data_byte();

    return 0;
}

static int sd_check_capacity() {
    INFO("Check SD card capacity with cmd58");
    for (int i = 0; i < 10; i++)
        recv_data_byte();

    char reply, cmd58[] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF};
    if (sd_exec_cmd(cmd58)) FATAL("cmd58 returns failure");
    INFO("SD card replies cmd58 with status 0x00");

    unsigned long payload;
    for (uint8_t i = 0; i < 4; i++) {
        reply = ((char*)&payload)[3 - i] = recv_data_byte();

        if (i == 0 && ((reply & 0XC0) == 0XC0))
            SD_CARD_TYPE = SD_CARD_TYPE_SDHC;
    }
    INFO("SD card replies cmd58 with payload 0x%.8x", payload);

    for (int i = 0; i < 100; i++)
        recv_data_byte();

    return 0;
}

static void sd_print_type() {
    switch (SD_CARD_TYPE) {
    case SD_CARD_TYPE_SDHC:
        INFO("SD card is high capacity SDHC card");
        break;
    default:
        FATAL("Unknown SD card type");
    }
}

static int sd_spi_reset() {
    INFO("Set CS and MOSI to 1 and toggle clock.");
    long control_base = SPI_BASE_ADDR;
    INFO("UART base address is 0x%x.", control_base);
    /* Keep chip select line high */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) &= ~(METAL_SPI_CSMODE_MASK);
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) |= METAL_SPI_CSMODE_HOLD;

    unsigned long i, rxdata;
    for (i = 0; i < 100; i++) {
        if (i % 20 == 0) INFO("    ... completed %d%c", i / 10 * 10, '%');

        send_data_byte(0xFF);
    }

    /* Keep chip select line low */
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSDEF) = 1;
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) &= ~(METAL_SPI_CSMODE_MASK);
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) |= METAL_SPI_CSMODE_HOLD;
    for (i = 0; i < 200000; i++);
    
    INFO("Set CS to 0 and send cmd0 through MOSI.");
    char cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    char reply = sd_exec_cmd(cmd0);

    while (reply != 0x01)
        reply = recv_data_byte();
    INFO("SD card replies cmd0 with 0x01");

    for(i = 0; i < 10; i++)
        recv_data_byte();
    return 0;
}

static int sd_spi_configure() {
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

static void sd_spi_set_clock(long baud_rate) {
    long cpu_clock_rate = 65000000;
    long div = (cpu_clock_rate / (2 * baud_rate)) - 1;
    if (div > METAL_SPI_SCKDIV_MASK)
        FATAL("SPI baud rate %lHz is too low", baud_rate);

    long control_base = SPI_BASE_ADDR;
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_SCKDIV) &= ~METAL_SPI_SCKDIV_MASK;
    METAL_SPI_REGW(METAL_SIFIVE_SPI0_SCKDIV) |= (div & METAL_SPI_SCKDIV_MASK);
}
