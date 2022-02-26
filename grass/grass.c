/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block
 * and spawn the first kernel process, sys_proc
 */

#include "egos.h"
#include "grass.h"

struct pcb_intf pcb;
struct earth *earth = (void*)EARTH_ADDR;
static void sys_proc_init();

int main() {
    SUCCESS("Enter the grass layer");

    proc_init();      // process control block
    timer_init();     // timer
    sys_proc_init();  // first kernel process

    void (*app_entry)(void*) = (void*)VADDR_START;
    pcb.timer_reset = timer_reset;
    pcb.proc_alloc = proc_alloc;
    pcb.proc_set_running = proc_set_running;
    pcb.proc_set_runnable = proc_set_runnable;

    earth->mmu_switch(GPID_PROCESS);
    proc_set_running(GPID_PROCESS);
    app_entry(&pcb);
    FATAL("Should never return to the grass kernel main()");

    return 0;
}

static int read_sys_proc(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

static void sys_proc_init() {
    int pid = proc_alloc();
    if (pid != GPID_PROCESS)
        FATAL("Process ID mismatch: %d != %d", pid, GPID_PROCESS);

    INFO("Load kernel process #%d: process spawn and kill", pid);
    struct block_store bs;
    bs.read = read_sys_proc;
    elf_load(pid, &bs, earth);
}
