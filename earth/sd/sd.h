#pragma once

#include "egos.h"

/* SD card library interface */
#define SD_BLOCK_SIZE 512

enum {
      SD_CARD_TYPE_SD1,
      SD_CARD_TYPE_SD2,
      SD_CARD_TYPE_SDHC,
      SD_CARD_TYPE_UNKNOWN
};
extern int SD_CARD_TYPE;

int sdinit();
int sdread(int offset, int nblock, char* dst);
int sdwrite(int offset, int nblock, char* src);

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

#define METAL_SIFIVE_SPI0_SCKDIV    0UL
#define METAL_SIFIVE_SPI0_SCKMODE   4UL
#define METAL_SIFIVE_SPI0_CSID      16UL
#define METAL_SIFIVE_SPI0_CSDEF     20UL
#define METAL_SIFIVE_SPI0_CSMODE    24UL
#define METAL_SIFIVE_SPI0_FMT       64UL
#define METAL_SIFIVE_SPI0_TXDATA    72UL
#define METAL_SIFIVE_SPI0_RXDATA    76UL
#define METAL_SIFIVE_SPI0_FCTRL     96UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define METAL_SPI_REG(offset) (SPI_BASE_ADDR + offset)
#define METAL_SPI_REGB(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned char*)METAL_SPI_REG(offset)))
#define METAL_SPI_REGW(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned int*)METAL_SPI_REG(offset)))
