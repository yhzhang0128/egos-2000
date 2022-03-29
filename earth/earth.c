/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * load the grass layer binary from disk and run it
 */


#include "egos.h"
#include "earth.h"

int earth_init();
void grass_load();
struct earth *earth = (void*)EARTH_STRUCT;

int main() {
    INFO("-----------------------------------");
    INFO("Start to initialize the earth layer");

    if (earth_init())
        FATAL("Failed at initializing the earth layer");

    INFO("Start to load the grass layer");
    grass_load();

    return 0;
}

int earth_init() {
    /* Initialize TTY */
    tty_init();
    earth->tty_intr = tty_intr;
    earth->tty_read = tty_read;
    earth->tty_write = printf;
    SUCCESS("Finished initializing the tty device");
    
    /* Initialize disk */
    disk_init();
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;
    SUCCESS("Finished initializing the disk device");

    /* Initialize CPU interrupt */
    intr_init();
    earth->intr_enable = intr_enable;
    earth->intr_disable = intr_disable;
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

    /* Initialize the logging functions */
    earth->log_info = log_info;
    earth->log_fatal = log_fatal;
    earth->log_success = log_success;
    earth->log_highlight = log_highlight;

    return 0;
}

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

void grass_load() {
    elf_load(0, grass_read, 0, NULL);

    /* call the grass kernel entry and never return */
    void (*grass_entry)() = (void*)GRASS_ENTRY;
    grass_entry();
}
