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

void earth_init();
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

void earth_init() {
    /* Initialize tty */
    tty_init();
    earth->tty_intr = tty_intr;
    earth->tty_read = tty_read;
    earth->tty_write = tty_write;
    
    earth->tty_printf = tty_printf;
    earth->tty_info = tty_info;
    earth->tty_fatal = tty_fatal;
    earth->tty_success = tty_success;
    earth->tty_critical = tty_critical;

    INFO("-----------------------------------");
    INFO("Start to initialize the earth layer");
    SUCCESS("Finished initializing the tty device");
    
    /* Initialize disk */
    disk_init();
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;
    SUCCESS("Finished initializing the disk device");

    /* Initialize CPU interrupt */
    intr_init();
    earth->intr_enable = intr_enable;
    earth->intr_register = intr_register;
    earth->excp_register = excp_register;
    SUCCESS("Finished initializing the CPU interrupts");

    /* Initialize CPU memory management unit */
    mmu_init();
    earth->mmu_free = mmu_free;
    earth->mmu_alloc = mmu_alloc;
    earth->mmu_map = mmu_map;
    earth->mmu_switch = mmu_switch;
    SUCCESS("Finished initializing the CPU memory management unit");
}
