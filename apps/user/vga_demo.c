/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple VGA demo
 * This app only runs on the Arty board. It reads the BMP image file
 * tools/screenshots/Bohr.bmp and displays the image through the VGA
 * video output onto a monitor. Both the image and the VGA resolution
 * are 800*600. The sifive_u machine in QEMU does not do video output.
 */

#include "app.h"

int main() {
    /* This is the tools/screenshots/Bohr.bmp loaded into boot ROM by mkfs.c. */
    char* rgb = (char*)(0x20400000 + 128 * 1024 * 5) + (56 + 800 * 600 * 3) - 3;

    /* The resolution for Arty board VGA video output is 800*600. */
    for (uint i = 0; i < 600; i++) {
        for (uint j = 0; j < 800; j++) {
            /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
            uint* pix = (uint*)(VGA_MMIO_START + 4 * (i * 800 + (800 - j)));
            /* Only 3 bytes are used for RGB and the 4th byte is unused. */
            *pix = rgb[0] | (((uint)rgb[1]) << 8) | (((uint)rgb[2]) << 16);
            rgb -= 3;
        }
    }
}
