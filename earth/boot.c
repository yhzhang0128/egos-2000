/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: bootloader
 * Initialize the tty device, disk device, MMU, and CPU interrupts.
 */

#include "process.h"

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
         * and you could ask AI/LLMs about the Bochs Dispi (Display Interface).
         * Your driver should setup the PCI ECAM for VGA, and then set the VGA
         * screen resolution to 800*600 pixels, each using 4 bytes for its RGB
         * information. Lastly, initialize all the pixels with white color. */

        /* Student's code ends here. */

        /* Student's code goes here (Ethernet & TCP/IP). */

        /* Use ETH_CTL_BASE as the BAR0 in Ethernet's PCIe configuration. Enable
         * the Ethernet controller's RXT0 (Receiver Timer Interrupt). Initialize
         * the MAC address register and the RX descriptor registers. Ensure that
         * every RX descriptor's address field is written with its corresponding
         * RX buffer's address. Read the descriptions for RCTL (Receive Control)
         * and decide which bits should be set to 1. */

        /* Student's code ends here. */

        grass_entry(core_id);
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (Multicore & Locks). */

        /* Refer to mmu_init() and intr_init(), and decide how to initialize
         * the CSRs for virtual memory and interrupts on this CPU core. */

        /* Reset the timer, release the boot lock, and run the wfi instruction.
         * After the next timer interrupt, this CPU core will enter the kernel,
         * and the kernel could schedule a process to run on this CPU core. */

        /* Student's code ends here. */
    }
}
