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
void timer_init();
void intr_init();

struct grass *grass = (void*)APPS_STACK_TOP;
struct earth *earth = (void*)GRASS_STACK_TOP;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init(uint core_id) {
    earth->platform = (core_id == 0)? ARTY : QEMU;

    tty_init();
    CRITICAL("--- Booting with %s, core #%u ---", earth->platform == ARTY? "Arty" : "QEMU", core_id);

    disk_init();
    SUCCESS("Finished initializing the tty and disk devices");

    mmu_init();
    timer_init();
    intr_init();
    SUCCESS("Finished initializing the MMU, timer and interrupts");
}

static int grass_read(uint block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

void non_boot(uint core_id) {
    SUCCESS("--- Core #%u starts running ---", core_id);
    *(uint*)(0x20800004) += 1;
    *(uint*)(0x20800000) = 0;  /* Release the boot lock */

    /* Student's code goes here (multi-core and atomic instruction) */
    while(1);
}

void boot(uint core_id) {
    /* Prepare the bss and data memory regions */
    memset(&bss_start, 0, (&bss_end - &bss_start));
    memcpy(&data_start, &data_rom, (&data_end - &data_start));

    /* Initialize the earth layer */
    earth_init(core_id);

    /* Load and enter the grass layer */
    elf_load(0, grass_read, 0, 0);

    uint mstatus, M_MODE = 3, S_MODE = 1; /* U_MODE = 0 */
    uint GRASS_MODE = (earth->translation == SOFT_TLB)? M_MODE : S_MODE;
    asm("csrr %0, mstatus" : "=r"(mstatus));
    asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (GRASS_MODE << 11) | (1 << 18)));

    asm("csrw mepc, %0" ::"r"(GRASS_ENTRY));
    asm("mret");
}
