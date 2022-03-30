#pragma once

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
int sdread(uint32_t offset, uint32_t nblock, char* dst);
int sdwrite(uint32_t offset, uint32_t nblock, char* src);

char sd_exec_cmd(char*);
char sd_exec_acmd(char*);
char recv_data_byte();
char send_data_byte(char);

#include "bus_spi.h"
#include "egos.h"
