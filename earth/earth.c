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
#include <string.h>

void tty_init();
void disk_init();
void intr_init();
void mmu_init();

struct grass *grass = (void*)APPS_STACK_TOP;
struct earth *earth = (void*)GRASS_STACK_TOP;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init() {
    int misa;
    asm("csrr %0, misa" : "=r"(misa));
    /* Arty board does not support the supervisor mode or page tables */
    #define MISA_SMODE (1 << 18)
    earth->platform = (misa & MISA_SMODE)? QEMU : ARTY;

    tty_init();
    CRITICAL("--- Booting on %s ---", earth->platform == QEMU? "QEMU" : "Arty");
    SUCCESS("Finished initializing the tty device");

    disk_init();
    SUCCESS("Finished initializing the disk device");

    mmu_init();
    SUCCESS("Finished initializing the memory management unit");

    intr_init();
    SUCCESS("Finished initializing and enabling the CPU interrupts");
}

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

int main() {
    /* Prepare the bss and data memory regions */
    memset(&bss_start, 0, (&bss_end - &bss_start));
    memcpy(&data_start, &data_rom, (&data_end - &data_start));

    /* Initialize the earth layer */
    earth_init();

    /* Setup a PMP region for the whole 4GB address space */
    asm("csrw pmpaddr0, %0" : : "r" (0x40000000));
    asm("csrw pmpcfg0, %0" : : "r" (0xF));

    /* Load and enter the grass layer */
    elf_load(0, grass_read, 0, 0);
    if (earth->translation == SOFT_TLB){
        /* No need to enter supervisor mode if using SOFT_TLB translation */
        void (*grass_entry)() = (void*)GRASS_ENTRY;
        grass_entry();
    } else {
        int mstatus;
        /* Enter the grass layer in supervisor mode for PAGE_TABLE translation */
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) | (1 << 18)));
        asm("csrw mepc, %0" ::"r"(GRASS_ENTRY));
        asm("mret");
    }
}
