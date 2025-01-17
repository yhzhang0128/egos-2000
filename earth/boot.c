/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: bootloader
 * Initialize the tty device, disk device, CPU MMU, and CPU interrupts
 */

#include "disk.h"
#include "egos.h"
#include "elf.h"
#include <string.h>

void tty_init();
void disk_init();
void mmu_init();
void intr_init(uint core_id);

struct grass* grass = (void*)GRASS_STRUCT_BASE;
struct earth* earth = (void*)EARTH_STRUCT_BASE;

void grass_entry();
void kernel_entry(uint);

void boot() {
    uint core_id, vendor_id;
    asm("csrr %0, mhartid" : "=r"(core_id));
    asm("csrr %0, mvendorid" : "=r"(vendor_id));
    earth->platform = (vendor_id == 666) ? ARTY : QEMU;

    /* Disable core#0 on QEMU because it is an E31 core without S-mode */
    /* See https://www.qemu.org/docs/master/system/riscv/sifive_u.html */
    if (earth->platform == QEMU && core_id == 0) {
        release(boot_lock);
        release(kernel_lock);
        while (1);
    }

    if (booted_core_cnt++ == 0) {
        /* The first booted core needs to do some more work */
        tty_init();
        CRITICAL("--- Booting on %s with core #%d ---",
                 earth->platform == ARTY ? "Arty" : "QEMU", core_id);

        disk_init();
        SUCCESS("Finished initializing the tty and disk devices");

        mmu_init();
        intr_init(core_id);
        SUCCESS("Finished initializing the MMU, timer and interrupts");

        uint mstatus, M_MODE = 3, S_MODE = 1; /* U_MODE = 0 */
        uint GRASS_MODE = (earth->translation == SOFT_TLB) ? M_MODE : S_MODE;
        asm("csrr %0, mstatus" : "=r"(mstatus));
        mstatus = (mstatus & ~(3 << 11)) | (GRASS_MODE << 11);
        asm("csrw mstatus, %0" ::"r"(mstatus));
        asm("csrw mepc, %0" ::"r"(grass_entry));
        asm("mret");
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (multi-core and atomic instruction) */

        /* Initialize the MMU and interrupts on this core */
        /* Read mmu_init() and intr_init(), and decide what to do here */

        /* Set core to idle, reset the timer, release the boot and kernel
         * locks, and then wait for the timer interrupt with while(1); */

        /* Student's code ends here. */
    }
}
