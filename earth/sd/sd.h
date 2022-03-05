#pragma once

/* SD card library interface */
#define SD_BLOCK_SIZE 512

int sdinit();
int sdread(uint32_t offset, uint32_t nblock, char* dst);
int sdwrite(uint32_t offset, uint32_t nblock, char* src);

enum {
      SD_CARD_TYPE_SD1,
      SD_CARD_TYPE_SD2,
      SD_CARD_TYPE_SDHC,
      SD_CARD_TYPE_UNKNOWN
};
extern int SD_CARD_TYPE;

struct metal_spi;
char recv_data_byte(struct metal_spi*);
char send_data_byte(struct metal_spi*, char);
char sd_exec_cmd(struct metal_spi *, char*);
char sd_exec_acmd(struct metal_spi *, char*);

#include "bus_spi.h"
