#pragma once

struct metal_spi;
char recv_data_byte(struct metal_spi*);
char send_data_byte(struct metal_spi*, char);
char sd_exec_cmd(struct metal_spi *, char*);
char sd_exec_acmd(struct metal_spi *, char*);

#include "bus_spi.h"
