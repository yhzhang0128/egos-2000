/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block and 
 * spawns some kernel processes, including file system and shell
 */

#include "egos.h"
#include "grass.h"

struct earth *earth = (void*)EARTH_ADDR;

static void fs_init();
static void intr_handler(int id);

int main() {
    SUCCESS("Enter the grass layer");

    proc_init();
    timer_init();
    fs_init();

    earth->mmu_switch(GPID_FS);
    earth->intr_enable();
    timer_reset();
    void (*app_entry)() = (void*)VADDR_START;
    app_entry();

    return 0;
}

static int read_fs_elf(int block_no, char* dst) {
    return earth->disk_read(FS_EXEC_START + block_no, 1, dst);
}

static void fs_init() {
    INFO("Load the file system as process #%d", GPID_FS);
    struct block_store bs;
    bs.read = read_fs_elf;
    elf_load(GPID_FS, &bs, earth);
}
