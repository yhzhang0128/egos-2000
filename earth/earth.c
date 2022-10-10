/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Initialize the bss and data segments;
 * Initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * Load the grass layer binary from disk and run it.
 */

#include "elf.h"
#include "disk.h"
#include "egos.h"

void tty_init();
void disk_init();
void intr_init();
void mmu_init();
struct earth *earth = (void*)GRASS_STACK_TOP;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init() {
    tty_init();
    CRITICAL("-----------------------------------");
    CRITICAL("Start to initialize the earth layer");
    SUCCESS("Finished initializing the tty device");
    
    disk_init();
    SUCCESS("Finished initializing the disk device");

    intr_init();
    SUCCESS("Finished initializing the CPU interrupts");

    mmu_init();
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
    elf_load(0, grass_read, 0, 0);

    if (earth->platform == ARTY){
        /* Arty board does not support supervisor mode */
        void (*grass_entry)() = (void*)GRASS_ENTRY;
        grass_entry();
    } else {
        /* QEMU supports supervisor mode */
        earth->intr_enable();

        int mstatus;
        /* Enter supervisor mode after mret */
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) ));
        /* Enter the grass layer after mret */
        asm("csrw mepc, %0" ::"r"(GRASS_ENTRY));
        asm("mret");
    }
}
