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
static struct earth earth;

int main() {
    INFO("-----------------------------------");
    INFO("Start to initialize the earth layer");

    if (earth_init())
        return -1;

    if (sizeof(earth) > 0x80)
        FATAL("Size of earth interface (%d bytes) exceeds 128 bytes", sizeof(earth));
    INFO("Put earth interface (%d bytes) at 0x%.8x", sizeof(earth), EARTH_STRUCT);
    memcpy((void*)EARTH_STRUCT, &earth, sizeof(earth));

    INFO("Start to load the grass layer");
    grass_load();

    return 0;
}

int earth_init() {
    /* Initialize TTY */
    if (tty_init())
        FATAL("Failed at initializing the tty device");
    earth.tty_read = tty_read;
    earth.tty_write = tty_write;
    SUCCESS("Finished initializing the tty device");
    
    /* Initialize disk */
    if (disk_init())
        FATAL("Failed at initializing the disk device");
    earth.disk_read = disk_read;
    earth.disk_write = disk_write;
    SUCCESS("Finished initializing the disk device");

    /* Initialize CPU interrupt */
    if (intr_init())
        FATAL("Failed at initializing the CPU interrupts");
    earth.intr_enable = intr_enable;
    earth.intr_disable = intr_disable;
    earth.intr_register = intr_register;
    earth.excp_register = excp_register;
    SUCCESS("Finished initializing the CPU interrupts");

    /* Initialize CPU memory management unit */
    if (mmu_init())
        FATAL("Failed at initializing the CPU interrupts");
    earth.mmu_free = mmu_free;
    earth.mmu_alloc = mmu_alloc;
    earth.mmu_map = mmu_map;
    earth.mmu_switch = mmu_switch;
    SUCCESS("Finished initializing the CPU memory management unit");

    /* Initialize the logging functions */
    earth.log.log_info = log_info;
    earth.log.log_highlight = log_highlight;
    earth.log.log_success = log_success;
    earth.log.log_fatal = log_fatal;

    return 0;
}

static int grass_read(int block_no, char* dst) {
    return earth.disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

void grass_load() {
    elf_load(0, grass_read, &earth);

    /* call the grass kernel entry and never return */
    void (*grass_entry)() = (void*)GRASS_ENTRY;
    grass_entry();
}
