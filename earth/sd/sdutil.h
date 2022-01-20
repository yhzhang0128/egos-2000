#ifndef SDLIB_UTIL_H
#define SDLIB_UTIL_H

#include <metal/spi.h>
#include <metal/machine.h>

char recv_data_byte(struct metal_spi*);
char send_data_byte(struct metal_spi*, char);
char sd_exec_cmd(struct metal_spi *, char*);
char sd_exec_acmd(struct metal_spi *, char*);

/* FE310 SPI interface */
#define METAL_SPI_SCKDIV_MASK 0xFFF

#define METAL_SPI_SCKMODE_PHA_SHIFT 0
#define METAL_SPI_SCKMODE_POL_SHIFT 1

#define METAL_SPI_CSMODE_MASK 3
#define METAL_SPI_CSMODE_AUTO 0
#define METAL_SPI_CSMODE_HOLD 2
#define METAL_SPI_CSMODE_OFF 3

#define METAL_SPI_PROTO_MASK 3
#define METAL_SPI_PROTO_SINGLE 0

#define METAL_SPI_ENDIAN_LSB 4

#define METAL_SPI_DISABLE_RX 8

#define METAL_SPI_FRAME_LEN_SHIFT 16
#define METAL_SPI_FRAME_LEN_MASK (0xF << METAL_SPI_FRAME_LEN_SHIFT)

#define METAL_SPI_TXDATA_FULL (1 << 31)
#define METAL_SPI_RXDATA_EMPTY (1 << 31)
#define METAL_SPI_TXMARK_MASK 7
#define METAL_SPI_TXWM 1
#define METAL_SPI_TXRXDATA_MASK (0xFF)

#define METAL_SPI_INTERVAL_SHIFT 16

#define METAL_SPI_CONTROL_IO 0
#define METAL_SPI_CONTROL_MAPPED 1


#define METAL_SPI_REG(offset) (((unsigned long)control_base + offset))
#define METAL_SPI_REGB(offset)                                                 \
    (__METAL_ACCESS_ONCE((__metal_io_u8 *)METAL_SPI_REG(offset)))
#define METAL_SPI_REGW(offset)                                                 \
    (__METAL_ACCESS_ONCE((__metal_io_u32 *)METAL_SPI_REG(offset)))


#endif
