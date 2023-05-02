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

static void platform_detect(int id) {
    earth->platform = ARTY;
    /* Skip the illegal store instruction */
    int mepc;
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrw mepc, %0" ::"r"(mepc + 4));
}

static void earth_init() {
    tty_init();
    CRITICAL("------------- Booting -------------");
    SUCCESS("Finished initializing the tty device");
    
    intr_init();
    SUCCESS("Finished initializing the CPU interrupts");

    /* Detect the hardware platform (Arty or QEMU) */
    earth->platform = QEMU;
    earth->excp_register(platform_detect);
    /* This memory access triggers an exception on Arty, but not QEMU */
    *(int*)(0x1000) = 1;

    disk_init();
    SUCCESS("Finished initializing the disk device");

    mmu_init();
    SUCCESS("Finished initializing the CPU memory management unit");
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

    /* Load and enter the grass layer */
    elf_load(0, grass_read, 0, 0);
    if (earth->translation == SOFT_TLB){
        /* No need to enter supervisor mode if using softTLB translation */
        void (*grass_entry)() = (void*)GRASS_ENTRY;
        grass_entry();
    } else {
        /* Enable machine-mode interrupt before entering supervisor mode */
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
