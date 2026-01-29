/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: bootloader
 * Initialize the tty device, disk device, MMU, and CPU interrupts.
 */

#include "egos.h"

void tty_init();
void disk_init();
void mmu_init();
void intr_init(uint core_id);
void grass_entry(uint core_id);

struct grass* grass = (void*)GRASS_STRUCT;
struct earth* earth = (void*)EARTH_STRUCT;

/* Student's code goes here (Ethernet & TCP/IP). */

/* Define an array of 128 RX descriptors and a buffer of 128*2048 bytes. Every
 * descriptor corresponds to a 2048-byte RX buffer for receiving from Ethernet.
 */

/* Student's code ends here. */

void boot() {
    uint core_id, vendor_id;
    asm("csrr %0, mhartid" : "=r"(core_id));
    asm("csrr %0, mvendorid" : "=r"(vendor_id));
    earth->platform = (vendor_id == 666) ? HARDWARE : QEMU;

    if (booted_core_cnt++ == 0) {
        /* The first booted core needs to do some more work. */
        tty_init();
        CRITICAL("--- Booting on %s with core #%d ---",
                 earth->platform == HARDWARE ? "Hardware" : "QEMU", core_id);

        disk_init();
        SUCCESS("Finished initializing the tty and disk devices");

        mmu_init();
        intr_init(core_id);
        SUCCESS("Finished initializing the MMU, timer and interrupts");

        /* Student's code goes here (I/O Device Driver). */

        /* Initialize QEMU's standard VGA device for apps/user/video_demo.c.
         * Start with https://www.qemu.org/docs/master/specs/standard-vga.html,
         * and you could ask ChatGPT about the Bochs Dispi (Display Interface).
         * Your driver should setup the PCI ECAM for VGA, and then set the VGA
         * screen resolution to 800*600 pixels, each using 4 bytes for its RGB
         * information. Lastly, initialize all the pixels with white color. */

        /* Student's code ends here. */

        /* Student's code goes here (Ethernet & TCP/IP). */

        /* Use ETH_CTL_BASE as the BAR0 in Ethernet's PCIe configuration. Enable
         * the Ethernet controller's RXT0 (Receiver Timer Interrupt). Initialize
         * the MAC address register (Receive Address Low/High) and the registers
         * for RX descriptors. Make sure the address field of each RX descriptor
         * is written by the corresponding RX buffer address. Lastly, set the EN
         * and SECRC bits of RCTL (Receive Control Register) to 1. */

        /* Student's code ends here. */

        grass_entry(core_id);
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (Multicore & Locks). */

        /* Initialize the MMU and interrupts on this CPU core.
         * Read mmu_init() and intr_init(), and decide what to do here. */

        /* Reset the timer, release the boot lock, and then hang the core
           by waiting for a timer interrupt using the wfi instruction. */

        /* Student's code ends here. */
    }
}
