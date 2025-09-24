/*
 * (C) 2025, Cornell University
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

struct grass* grass = (void*)GRASS_STRUCT_BASE;
struct earth* earth = (void*)EARTH_STRUCT_BASE;

void hang();
void grass_entry();
void core_set_idle(uint core);

void boot() {
    uint core_id, vendor_id;
    asm("csrr %0, mhartid" : "=r"(core_id));
    asm("csrr %0, mvendorid" : "=r"(vendor_id));
    earth->platform = (vendor_id == 666) ? HARDWARE : QEMU;

    /* Disable core#0 on QEMU because it is an E31 core without S-mode. */
    if (earth->platform == QEMU && core_id == 0) {
        release(boot_lock);
        hang();
    }

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

        grass_entry();
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (Multicore & Locks). */

        /* Initialize the MMU and interrupts on this CPU core.
         * Read mmu_init() and intr_init(), and decide what to do here. */

        /* Call function core_set_idle, reset the timer, release the boot
         * lock, and wait for a timer interrupt using the wfi instruction. */

        /* Student's code ends here. */
    }
}
