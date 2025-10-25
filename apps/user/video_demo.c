/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple video output demo
 * This app reads the BMP image file tools/images/Bohr.bmp and displays
 * this image onto a monitor through VGA or HDMI. This app works on the
 * FPGA boards, while QEMU requires some more code initializing the VGA
 * device. See QEMU_GRAPHIC in Makefile and QEMU's official document on
 * VGA device: https://www.qemu.org/docs/master/specs/standard-vga.html
 */

#include "app.h"
#include "disk.h"

#define HRES 800
#define VRES 600

int main() {
    /* This is tools/images/Bohr.bmp loaded into the ROM image by mkfs.c. */
    char* rgb = (char*)(FLASH_ROM_BASE + EGOS_BIN_MAX_NBYTE * 5) +
                (56 + HRES * VRES * 3) - 3;

    /* The resolution for VGA/HDMI video output is 800*600. */
    for (uint i = 0; i < VRES; i++) {
        for (uint j = 0; j < HRES; j++) {
            /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
            uint* pix = (uint*)(VIDEO_FRAME_BASE + 4 * (i * HRES + (HRES - j)));

            /* Only 3 bytes are used for RGB and the 4th byte is unused. */
            *pix = rgb[0] | (((uint)rgb[1]) << 8) | (((uint)rgb[2]) << 16);
            rgb -= 3;
        }
    }
}
