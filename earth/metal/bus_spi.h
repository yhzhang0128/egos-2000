/* 
 * Definitions for controlling SPI in FE310
 * copied from the Freedom Metal library: https://github.com/sifive/freedom-metal
 */
#pragma once

#define SPI_BASE_ADDR 0x10024000

#define METAL_SPI_SCKDIV_MASK 0xFFF
#define METAL_SPI_SCKMODE_PHA_SHIFT 0
#define METAL_SPI_SCKMODE_POL_SHIFT 1

#define METAL_SPI_CSMODE_MASK     3
#define METAL_SPI_CSMODE_AUTO     0
#define METAL_SPI_CSMODE_HOLD     2
#define METAL_SPI_CSMODE_OFF      3

#define METAL_SPI_PROTO_MASK      3
#define METAL_SPI_PROTO_SINGLE    0

#define METAL_SPI_ENDIAN_LSB      4

#define METAL_SPI_DISABLE_RX      8

#define METAL_SPI_FRAME_LEN_SHIFT 16
#define METAL_SPI_FRAME_LEN_MASK  (0xF << METAL_SPI_FRAME_LEN_SHIFT)

#define METAL_SPI_TXDATA_FULL     (1 << 31)
#define METAL_SPI_RXDATA_EMPTY    (1 << 31)
#define METAL_SPI_TXMARK_MASK     7
#define METAL_SPI_TXWM            1
#define METAL_SPI_TXRXDATA_MASK   (0xFF)

#define METAL_SPI_INTERVAL_SHIFT  16

#define METAL_SPI_CONTROL_IO      0
#define METAL_SPI_CONTROL_MAPPED  1

#define METAL_SIFIVE_SPI0_SCKDIV  0UL
#define METAL_SIFIVE_SPI0_FMT     64UL
#define METAL_SIFIVE_SPI0_CSID    16UL
#define METAL_SIFIVE_SPI0_FCTRL   96UL
#define METAL_SIFIVE_SPI0_CSDEF   20UL
#define METAL_SIFIVE_SPI0_CSMODE  24UL
#define METAL_SIFIVE_SPI0_TXDATA  72UL
#define METAL_SIFIVE_SPI0_RXDATA  76UL
#define METAL_SIFIVE_SPI0_SCKMODE 4UL


#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define METAL_SPI_REG(offset) (((unsigned long)control_base + offset))
#define METAL_SPI_REGB(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned char*)METAL_SPI_REG(offset)))
#define METAL_SPI_REGW(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned int*)METAL_SPI_REG(offset)))
