#pragma once

struct metal_spi;
char recv_data_byte(struct metal_spi*);
char send_data_byte(struct metal_spi*, char);
char sd_exec_cmd(struct metal_spi *, char*);
char sd_exec_acmd(struct metal_spi *, char*);

/* FE310 SPI interface */
#define SPI_BASE_ADDR 0x10024000

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

#define METAL_SIFIVE_SPI0_FMT 64UL
#define METAL_SIFIVE_SPI0_CSID 16UL
#define METAL_SIFIVE_SPI0_FCTRL 96UL
#define METAL_SIFIVE_SPI0_CSDEF 20UL
#define METAL_SIFIVE_SPI0_CSMODE 24UL
#define METAL_SIFIVE_SPI0_TXDATA 72UL
#define METAL_SIFIVE_SPI0_RXDATA 76UL
#define METAL_SIFIVE_SPI0_SCKMODE 4UL

#define __METAL_ACCESS_ONCE(x) (*(__typeof__(*x) volatile *)(x))
#define METAL_SPI_REG(offset) (((unsigned long)control_base + offset))
#define METAL_SPI_REGB(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned char*)METAL_SPI_REG(offset)))
#define METAL_SPI_REGW(offset)                                                 \
    (__METAL_ACCESS_ONCE((unsigned int*)METAL_SPI_REG(offset)))


/* definitions for controlling SPI, copied from metal/spi.h */
struct metal_spi_config {
    /*! @brief The protocol for the SPI transfer */
    enum { METAL_SPI_SINGLE, METAL_SPI_DUAL, METAL_SPI_QUAD } protocol;

    /*! @brief The polarity of the SPI transfer, equivalent to CPOL */
    unsigned int polarity : 1;
    /*! @brief The phase of the SPI transfer, equivalent to CPHA */
    unsigned int phase : 1;
    /*! @brief The endianness of the SPI transfer */
    unsigned int little_endian : 1;
    /*! @brief The active state of the chip select line */
    unsigned int cs_active_high : 1;
    /*! @brief The chip select ID to activate for the SPI transfer */
    unsigned int csid;
    /*! @brief The spi command frame number (cycles = num * frame_len) */
    unsigned int cmd_num;
    /*! @brief The spi address frame number */
    unsigned int addr_num;
    /*! @brief The spi dummy frame number */
    unsigned int dummy_num;
    /*! @brief The Dual/Quad spi mode selection.*/
    enum {
        MULTI_WIRE_ALL,
        MULTI_WIRE_DATA_ONLY,
        MULTI_WIRE_ADDR_DATA
    } multi_wire;
};

struct metal_spi_vtable {
    void (*init)(struct metal_spi *spi, int baud_rate);
    int (*transfer)(struct metal_spi *spi, struct metal_spi_config *config,
                    size_t len, char *tx_buf, char *rx_buf);
    int (*get_baud_rate)(struct metal_spi *spi);
    int (*set_baud_rate)(struct metal_spi *spi, int baud_rate);
};

struct metal_spi {
    const struct metal_spi_vtable *vtable;
};

struct metal_spi *metal_spi_get_device(unsigned int device_num);
__inline__ int metal_spi_set_baud_rate(struct metal_spi *spi, int baud_rate) {
    return spi->vtable->set_baud_rate(spi, baud_rate);
}
