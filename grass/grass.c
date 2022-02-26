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

    timer_init();
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

static void intr_handler(int id) {
    if (id == INTR_ID_TMR) {
        timer_reset();
    } else if (id == INTR_ID_SOFT) {
        struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;
        sc->type = SYS_UNUSED;
        *((int*)RISCV_CLINT0_MSIP_BASE) = 0;

        INFO("Got system call #%d with arg %d", sc->type, sc->args.exit.status);        
    } else {
        FATAL("Got unknown interrupt #%d", id);
    }
}
