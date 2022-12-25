#pragma once

#include "egos.h"

char recv_data_byte();
char send_data_byte(char);

char sd_exec_cmd(char*);
char sd_exec_acmd(char*);

void sdinit();
int sdread(int offset, int nblock, char* dst);
int sdwrite(int offset, int nblock, char* src);

/* definitions for controlling SPI1 in FE310
 * see chapter19 of the SiFive FE310-G002 Manual
 */

#define SPI1_BASE     0x10024000UL

#define SPI1_SCKDIV   0UL
#define SPI1_SCKMODE  4UL
#define SPI1_CSID     16UL
#define SPI1_CSDEF    20UL
#define SPI1_CSMODE   24UL
#define SPI1_FMT      64UL
#define SPI1_TXDATA   72UL
#define SPI1_RXDATA   76UL
#define SPI1_FCTRL    96UL
