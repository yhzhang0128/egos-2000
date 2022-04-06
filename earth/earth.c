/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * load the grass layer binary from disk and run it
 */


#include "earth.h"

void earth_init();
struct earth *earth = (void*)GRASS_STACK_TOP;

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

extern char metal_segment_data_source_start;
extern char metal_segment_data_target_start;
extern char metal_segment_data_target_end;

extern char metal_segment_bss_target_start;
extern char metal_segment_bss_target_end;

int main() {
    char* data_src = &metal_segment_data_source_start;
    char* data_dst = &metal_segment_data_target_start;
    int size = &metal_segment_data_target_end - data_dst;
    for (int i = 0; i < size; i++) data_dst[i] = data_src[i];

    char* bss = &metal_segment_bss_target_start;
    int bss_size = &metal_segment_bss_target_end - bss;
    for (int i = 0; i < bss_size; i++) bss[i] = 0;
    
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
