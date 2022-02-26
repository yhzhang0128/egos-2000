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

    fs_init();
    earth->mmu_switch(PID_FS);

    earth->intr_register(intr_handler);
    earth->intr_enable();
    timer_reset();
    
    /* call the shell application entry and never return */
    void (*app_entry)() = (void*)VADDR_START;
    app_entry();

    return 0;
}

static int read_fs_elf(int block_no, char* dst) {
    return earth->disk_read(FS_EXEC_START + block_no, 1, dst);
}

static void fs_init() {
    INFO("Load the file system as process #%d", PID_FS);
    struct block_store bs;
    bs.read = read_fs_elf;
    elf_load(PID_FS, &bs, earth);
}



static int intr_cnt;
static void intr_handler(int id) {
    long long mtime = timer_reset();
    intr_cnt++;
    if (intr_cnt % 20 == 0)
        INFO("Interrupt count: %d, current intr id=%d, mtime=%lld", intr_cnt, id, mtime);
}
