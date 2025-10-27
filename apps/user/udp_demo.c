/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple UDP demo
 * This app sends the HELLO_MSG string below to a destination IP+UDP
 * port (i.e, the dest_ip and dest_udp_port below); This app is an
 * example of kernel-bypass networking because network communication
 * is fully handled within this app while the kernel is not involved.
 */

#include "app.h"

#define HELLO_MSG "Hello from egos-2000!\n\r"

/* Define the Mac addresses, IP addresses and UDP ports. */
#define LOCAL_MAC           {0x10, 0xe2, 0xd5, 0x00, 0x00, 0x00}
#define DEST_MAC            {0x98, 0x48, 0x27, 0x51, 0x53, 0x1e}
#define IPTOINT(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

uint local_ip       = IPTOINT(192, 168, 1, 50);
uint local_udp_port = 8001;
uint dest_ip        = IPTOINT(192, 168, 0, 212);
uint dest_udp_port  = 8002;

/* Define the data structures for an Ethernet frame. */
struct ethernet_header {
    uchar destmac[6];
    uchar srcmac[6];
    ushort ethertype;
} __attribute__((packed));

struct ip_header {
    uchar version;
    uchar diff_services;
    ushort total_length;
    ushort identification;
    ushort fragment_offset;
    uchar ttl;
    uchar proto;
    ushort checksum;
    uint src_ip;
    uint dst_ip;
} __attribute__((packed));

struct udp_header {
    ushort src_port;
    ushort dst_port;
    ushort length;
    ushort checksum;
} __attribute__((packed));

struct ethernet_frame {
    struct ethernet_header eth;
    struct ip_header ip;
    struct udp_header udp;
    char payload[sizeof(HELLO_MSG)];
} __attribute__((packed));

struct checksum_fields {
    uint src_ip;
    uint dst_ip;
    uchar zero;
    uchar proto;
    ushort length;
} __attribute__((packed));

/* Declare two helper functions for checksum. */
static uint crc32(const uchar* message, uint len) {
    uint byte, mask, crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        byte = message[i];
        crc  = crc ^ byte;
        for (int j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc  = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

static ushort checksum(uint r, char* ptr, uint length, int complete) {
    length >>= 1;

    for (int i = 0; i < length; i++)
        r += ((uint)(ptr[2 * i]) << 8) | (uint)(ptr[2 * i + 1]);

    /* Add overflows */
    while (r >> 16) r = (r & 0xffff) + (r >> 16);

    if (complete) {
        r = (~r) & 0xffff;
        if (r == 0) r = 0xffff;
    }
    return r;
}

int main() {
    /* Initialize an Ethernet frame. */
    /* clang-format off */
    struct ethernet_frame eth_frame = {
        .eth = {
            .srcmac          = LOCAL_MAC,
            .destmac         = DEST_MAC,
            .ethertype       = __builtin_bswap16(0x0800) /* ETHERTYPE_IP */
        },
        .ip = {
            .version         = 0x45, /* IP_IPV4 */
            .diff_services   = 0,
            .total_length    = __builtin_bswap16(sizeof(struct ip_header) + sizeof(struct udp_header) + sizeof(HELLO_MSG)),
            .identification  = __builtin_bswap16(0),
            .fragment_offset = __builtin_bswap16(0x4000), /* IP_DONT_FRAGMENT */
            .ttl             = 64,
            .proto           = 0x11, /* IP_PROTO_UDP */
            .src_ip          = __builtin_bswap32(local_ip),
            .dst_ip          = __builtin_bswap32(dest_ip)
        },
        .udp = {
            .src_port        = __builtin_bswap16(local_udp_port),
            .dst_port        = __builtin_bswap16(dest_udp_port),
            .length          = __builtin_bswap16(sizeof(struct udp_header) + sizeof(HELLO_MSG)),
        }
    };
    /* clang-format on */

    /* Setup payload. */
    if (sizeof(HELLO_MSG) & 1) FATAL("Please send a message with even length");
    memcpy(eth_frame.payload, HELLO_MSG, sizeof(HELLO_MSG));

    /* Calculate the IP checksum. */
    eth_frame.ip.checksum = __builtin_bswap16(
        checksum(0, (void*)&eth_frame.ip, sizeof(struct ip_header), 1));

    /* Calculate the UDP checksum. */
    struct checksum_fields check = {.src_ip = eth_frame.ip.src_ip,
                                    .dst_ip = eth_frame.ip.dst_ip,
                                    .zero   = 0,
                                    .proto  = eth_frame.ip.proto,
                                    .length = eth_frame.udp.length};
    uint r = checksum(0, (void*)&check, sizeof(struct checksum_fields), 0);
    eth_frame.udp.checksum = __builtin_bswap16(
        checksum(r, (void*)&eth_frame.udp,
                 sizeof(struct udp_header) + sizeof(HELLO_MSG), 1));

    /* Send out the Ethernet frame. */
    if (earth->platform == HARDWARE) {
        /* LiteX's liteeth Ethernet Controller. */
        char* txbuffer = (void*)(NIC_TX_BUFFER);
        /* LiteX ETHMAC provides 2 TX slots. */
        /* TX slot#0 is at 0x90001000 (txbuffer). */
        /* TX slot#1 is at 0x90001800 (not used). */
        memcpy(txbuffer, &eth_frame, sizeof(struct ethernet_frame));

        /* CRC32 is another checksum for the LiteX ETHMAC device. */
        uint txlen                   = sizeof(struct ethernet_frame);
        *((uint*)(txbuffer + txlen)) = crc32(&txbuffer[8], txlen - 8);

        while (!(REGW(NIC_BASE, 0x1C)));
        REGW(NIC_BASE, 0x24) = 0;
        REGW(NIC_BASE, 0x28) = txlen + sizeof(uint) /* crc32 */;
        REGW(NIC_BASE, 0x18) = 1;
    } else {
        /* Intel 82540EM Gigabit Ethernet Controller. */
        REGW(NIC_PCI_ECAM, 0x4)  = 6;
        REGW(NIC_PCI_ECAM, 0x10) = NIC_BASE;

        static char txbuffer[sizeof(struct ethernet_frame)];
        memcpy(txbuffer, &eth_frame, sizeof(struct ethernet_frame));

        static __attribute__((aligned(16))) char txdesc[16];
        REGW(txdesc, 0)  = (uint)txbuffer;       /* addr   */
        REGB(txdesc, 11) = REGB(txdesc, 11) | 5; /* cmd    */
        REGB(txdesc, 12) = REGB(txdesc, 12) | 5; /* status */

        REGW(NIC_BASE, 0x3800) = (uint)&txdesc;
        REGW(NIC_BASE, 0x3808) = 16;       /* tx descriptor length */
        REGW(NIC_BASE, 0x3818) = 0;        /* tx descriptor tail */
        REGW(NIC_BASE, 0x410)  = 0x60100A; /* tx inter-packet gap time */
        REGW(NIC_BASE, 0x400) |= 0x4010A;  /* tx control */

        while (!(REGB(txdesc, 12) & 1));
        REGB(txdesc, 12)       = REGB(txdesc, 12) & ~1;
        REGB(txdesc, 8)        = sizeof(struct ethernet_frame);
        REGW(NIC_BASE, 0x3818) = 1;
    }

    return 0;
}
