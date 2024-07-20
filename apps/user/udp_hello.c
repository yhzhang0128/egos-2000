#include "app.h"
#include <string.h>

#define HELLO_MSG "Hello from egos-2000!\n\r"

#define LOCAL_MAC {0x10, 0xe2, 0xd5, 0x00, 0x00, 0x00}
#define DEST_MAC  {0x98, 0x48, 0x27, 0x51, 0x53, 0x1e}

#define IPTOINT(a, b, c, d) ((a << 24)|(b << 16)|(c << 8)|d)
static uint local_ip = IPTOINT(192, 168, 1, 50);
static uint dest_ip  = IPTOINT(192, 168, 0, 212);
static uint local_udp_port = 8001, dest_udp_port = 8002;

#define bswap_16(x) ((x<<8) | (x>>8))
#define bswap_32(x) (x>>24) | ((x>>8)&0xff00) | ((x<<8)&0xff0000) | (x<<24)

static uint16_t htons(uint16_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_16(n) : n;
}

static uint32_t htonl(uint32_t n)
{
    union { int i; char c; } u = { 1 };
    return u.c ? bswap_32(n) : n;
}

unsigned int crc32(const unsigned char *message, unsigned int len) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i < len) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

static unsigned short ip_checksum(unsigned int r, void *buffer, unsigned int length, int complete)
{
    unsigned char *ptr;
    unsigned int i;

    ptr = (unsigned char *)buffer;
    length >>= 1;

    for(i=0;i<length;i++)
        r += ((unsigned int)(ptr[2*i]) << 8)|(unsigned int)(ptr[2*i+1]) ;

    /* Add overflows */
    while(r >> 16)
        r = (r & 0xffff) + (r >> 16);

    if(complete) {
        r = ~r;
        r &= 0xffff;
        if(r == 0) r = 0xffff;
    }
    return r;
}

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

struct udp_frame {
    struct ip_header ip;
    struct udp_header udp;
    char payload[sizeof(HELLO_MSG)];
} __attribute__((packed));

struct ethernet_frame {
    struct ethernet_header eth_header;
    struct udp_frame frame;
} __attribute__((packed));

struct header_for_checksum {
    uint src_ip;
    uint dst_ip;
    uchar zero;
    uchar proto;
    ushort length;
} __attribute__((packed));

int main() {
    struct ethernet_header eth_header = {
        .srcmac = LOCAL_MAC,
        .destmac = DEST_MAC,
        .ethertype = htons(0x0800) /* ETHERTYPE_IP */
    };

    struct udp_frame frame;
    struct header_for_checksum check;
    uint length = sizeof(HELLO_MSG);
    memcpy(frame.payload, HELLO_MSG, length);
    frame.ip.version = 0x45; /* IP_IPV4 */
    frame.ip.diff_services = 0;
    frame.ip.total_length = htons(sizeof(struct udp_frame));
    frame.ip.identification = htons(0);
    frame.ip.fragment_offset = htons(0x4000); /* IP_DONT_FRAGMENT */
    frame.ip.ttl = 64;
    check.proto = frame.ip.proto = 0x11; /* IP_PROTO_UDP */
    frame.ip.checksum = 0;
    check.src_ip = frame.ip.src_ip = htonl(local_ip);
    check.dst_ip = frame.ip.dst_ip = htonl(dest_ip);
    frame.ip.checksum = htons(ip_checksum(0, &frame.ip, sizeof(struct ip_header), 1));

    frame.udp.src_port = htons(local_udp_port);
    frame.udp.dst_port = htons(dest_udp_port);
    check.length = frame.udp.length = htons(length + sizeof(struct udp_header));
    frame.udp.checksum = 0;

    check.zero = 0;
    uint r = ip_checksum(0, &check, sizeof(struct header_for_checksum), 0);
    if(length & 1) FATAL("Message needs to have even length");
    r = ip_checksum(r, &frame.udp, sizeof(struct udp_header)+length, 1);
    frame.udp.checksum = htons(r);

    #define ETHMAC_RX_BASE        0x90000000
    #define ETHMAC_RX_SLOTS       2
    #define ETHMAC_SLOT_SIZE      2048
    char* txbuffer = (void*)(ETHMAC_RX_BASE + ETHMAC_SLOT_SIZE * ETHMAC_RX_SLOTS);
    struct ethernet_frame eth_frame = {
        .eth_header = eth_header,
        .frame = frame
    };
    INFO("Sending a packet of %u bytes", sizeof(struct ethernet_frame));
    memcpy(txbuffer, &eth_frame, sizeof(struct ethernet_frame));

    unsigned int crc, txlen = sizeof(struct ethernet_frame);
    crc = crc32(&txbuffer[8], txlen-8);
    txbuffer[txlen  ] = (crc & 0xff);
    txbuffer[txlen+1] = (crc & 0xff00) >> 8;
    txbuffer[txlen+2] = (crc & 0xff0000) >> 16;
    txbuffer[txlen+3] = (crc & 0xff000000) >> 24;
    txlen += 4;

    #define ETHMAC_BASE           0xF0002000
    #define ETHMAC_START_WRITE    0x18
    #define ETHMAC_READY          0x1C
    #define ETHMAC_SLOT_WRITE     0x24
    #define ETHMAC_SLOT_LEN_WRITE 0x28

    while(!(REGW(ETHMAC_BASE, ETHMAC_READY)));
    REGW(ETHMAC_BASE, ETHMAC_SLOT_WRITE) = 0; /* TX slot#0 */
    REGW(ETHMAC_BASE, ETHMAC_SLOT_LEN_WRITE) = txlen;
    REGW(ETHMAC_BASE, ETHMAC_START_WRITE) = 1;
}
