/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple video output demo
 * This app only runs on the FPGA boards. It reads the BMP image file
 * tools/screenshots/Bohr.bmp and displays the image through VGA/HDMI
 * video output onto a monitor. Both the image and VGA/HDMI output are
 * 800*600 pixels. The sifive_u machine in QEMU does not support video.
 */

#include "app.h"
#include "disk.h"

#define HRES 800
#define VRES 600

int main() {
    /* This is tools/screenshots/Bohr.bmp loaded into the boot ROM by mkfs.c. */
    char* rgb = (char*)(0x20400000 + EGOS_BIN_MAX_NBYTE * 5) +
                (56 + HRES * VRES * 3) - 3;

    /* The resolution for VGA/HDMI video output is 800*600. */
    for (uint i = 0; i < VRES; i++) {
        for (uint j = 0; j < HRES; j++) {
            /* Every 4 bytes from VGA_MMIO_START correspond to a pixel. */
            uint* pix = (uint*)(VGA_HDMI_BASE + 4 * (i * HRES + (HRES - j)));

            /* Only 3 bytes are used for RGB and the 4th byte is unused. */
            *pix = rgb[0] | (((uint)rgb[1]) << 8) | (((uint)rgb[2]) << 16);
            rgb -= 3;
        }
    }
}
