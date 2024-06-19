/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: Initialize the bss and data segments;
 * Initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * Load the grass layer binary from disk and run it.
 */

#include "disk.h"
#include "egos.h"
#include "elf.h"
#include <string.h>

void tty_init();
void disk_init();
void mmu_init();
void intr_init(uint core_id);

struct grass *grass = (void*)APPS_STACK_TOP;
struct earth *earth = (void*)GRASS_STACK_TOP;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init(uint core_id) {
    earth->platform = (core_id == 0)? ARTY : QEMU;

    tty_init();
    CRITICAL("--- Booting on %s with core #%u ---", earth->platform == ARTY? "Arty" : "QEMU", core_id);

    disk_init();
    SUCCESS("Finished initializing the tty and disk devices");

    mmu_init();
    intr_init(core_id);
    SUCCESS("Finished initializing the MMU, timer and interrupts");
}

static int grass_read(uint block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

/* Makefile defines KERNEL_ENTRY which is parsed from grass.elf */
void (*kernel_entry)(uint) = (void*)KERNEL_ENTRY;

void boot(uint core_id, uint booted_core_cnt) {
    if (core_id == 0 || booted_core_cnt == 0) {
        /* Zero out the bss region and copy the data region from ROM */
        memset(&bss_start, 0, (&bss_end - &bss_start));
        memcpy(&data_start, &data_rom, (&data_end - &data_start));

        /* Initialize the earth layer */
        earth_init(core_id);
        earth->booted_core_cnt = 1;

        /* Load and enter the grass layer */
        elf_load(0, grass_read, 0, 0);

        uint mstatus, M_MODE = 3, S_MODE = 1; /* U_MODE = 0 */
        uint GRASS_MODE = (earth->translation == SOFT_TLB)? M_MODE : S_MODE;
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (GRASS_MODE << 11) | (1 << 18)));

        asm("csrw mepc, %0" ::"r"(GRASS_ENTRY));
        asm("mret");
    } else {
        /* This is not the first booted core */
        SUCCESS("--- Core #%u starts running ---", core_id);
        earth->booted_core_cnt++;

        /* Student's code goes here (multi-core and atomic instruction) */

        /* Initialize the MMU and interrupts on this core */
        /* Enter the kernel_entry and mock a timer interrupt */
        while(1);

        /* Student's code ends here. */
    }
}
