#pragma once

#include "egos.h"

/* SD card library interface */
#define SD_BLOCK_SIZE 512
#define uint32_t unsigned int

enum {
      SD_CARD_TYPE_SD1,
      SD_CARD_TYPE_SD2,
      SD_CARD_TYPE_SDHC,
      SD_CARD_TYPE_UNKNOWN
};
extern int SD_CARD_TYPE;

int sdinit();
int sdread(uint32_t offset, uint32_t nblock, char* dst);
int sdwrite(uint32_t offset, uint32_t nblock, char* src);

char sd_exec_cmd(char*);
char sd_exec_acmd(char*);
char recv_data_byte();
char send_data_byte(char);

/* 
 * Definitions for controlling SPI in FE310
 * copied from the Freedom Metal library: 
 * https://github.com/sifive/freedom-metal
 */

#define SPI_BASE_ADDR               0x10024000UL

#define METAL_SPI_SCKDIV_MASK       0xFFF
#define METAL_SPI_SCKMODE_PHA_SHIFT 0
#define METAL_SPI_SCKMODE_POL_SHIFT 1

#define METAL_SPI_CSMODE_MASK       3
#define METAL_SPI_CSMODE_AUTO       0
#define METAL_SPI_CSMODE_HOLD       2
#define METAL_SPI_CSMODE_OFF        3

#define METAL_SPI_PROTO_MASK        3
#define METAL_SPI_PROTO_SINGLE      0

#define METAL_SPI_ENDIAN_LSB        4

#define METAL_SPI_DISABLE_RX        8

#define METAL_SPI_FRAME_LEN_SHIFT   16
#define METAL_SPI_FRAME_LEN_MASK    (0xF << METAL_SPI_FRAME_LEN_SHIFT)

#define METAL_SPI_TXDATA_FULL       (1 << 31)
#define METAL_SPI_RXDATA_EMPTY      (1 << 31)
#define METAL_SPI_TXMARK_MASK       7
#define METAL_SPI_TXWM              1
#define METAL_SPI_TXRXDATA_MASK     (0xFF)

#define METAL_SPI_INTERVAL_SHIFT    16

#define METAL_SPI_CONTROL_IO        0
#define METAL_SPI_CONTROL_MAPPED    1

#define METAL_SIFIVE_SPI0_SCKDIV    0UL
#define METAL_SIFIVE_SPI0_FMT       64UL
#define METAL_SIFIVE_SPI0_CSID      16UL
#define METAL_SIFIVE_SPI0_FCTRL     96UL
#define METAL_SIFIVE_SPI0_CSDEF     20UL
#define METAL_SIFIVE_SPI0_CSMODE    24UL
#define METAL_SIFIVE_SPI0_TXDATA    72UL
#define METAL_SIFIVE_SPI0_RXDATA    76UL
#define METAL_SIFIVE_SPI0_SCKMODE   4UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define METAL_SPI_REG(offset) (SPI_BASE_ADDR + offset)
#define METAL_SPI_REGB(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned char*)METAL_SPI_REG(offset)))
#define METAL_SPI_REGW(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned int*)METAL_SPI_REG(offset)))
