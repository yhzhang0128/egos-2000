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
static void timer_handler(int id, void* arg);

int main() {
    SUCCESS("Enter the grass layer");

    fs_init();
    earth->mmu_switch(PID_FS);

    earth->intr_register(TIMER_INTR_ID, timer_handler);
    earth->intr_enable();
    
    /* call the shell application entry and never return */
    void (*app_entry)() = (void*)VADDR_START;
    app_entry();

    return 0;
}

static int read_fs_elf(int block_no, int nblocks, char* dst) {
    return earth->disk_read(FS_EXEC_START + block_no, nblocks, dst);    
}

static void fs_init() {
    INFO("Load the file system as process #%d", PID_FS);
    struct block_store bs;
    bs.read = read_fs_elf;
    elf_load(PID_FS, &bs, earth);
}

static int timer_cnt;
static void timer_handler(int id, void* arg) {
    timer_cnt++;
    if (timer_cnt % 20 == 0)
        INFO("Timer interrupt count: %d", timer_cnt);
}

