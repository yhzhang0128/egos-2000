/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Initialize the bss and data segments;
 * Initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * Load the grass layer binary from disk and run it.
 */

#include "earth.h"

struct earth *earth = (void*)GRASS_STACK_TOP;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init() {
    tty_init(earth);
    INFO("-----------------------------------");
    SUCCESS("Finished initializing the tty device");
    
    disk_init(earth);
    SUCCESS("Finished initializing the disk device");

    intr_init(earth);
    SUCCESS("Finished initializing the CPU interrupts");

    mmu_init(earth);
    SUCCESS("Finished initializing the CPU memory management unit");
}

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

int main() {
    for (int i = 0; i < (&bss_end - &bss_start); i++)
        ((char*)&bss_start)[i] = 0;
    for (int i = 0; i < (&data_end - &data_start); i++)
        ((char*)&data_start)[i] = ((char*)&data_rom)[i];

    earth_init();
    INFO("Start to load the grass layer");
    elf_load(0, grass_read, 0, NULL);
    void (*grass_entry)() = (void*)GRASS_ENTRY;
    grass_entry();
}
