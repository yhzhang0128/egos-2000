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
struct earth *earth = (void*)EARTH_STRUCT;
static void sys_proc_init();

int main() {
    SUCCESS("Enter the grass layer");

    proc_init();
    timer_init();
    sys_proc_init();

    /* Enter the first kernel process sys_proc */
    void (*sys_proc_entry)(void*) = (void*)APPS_ENTRY;
    earth->mmu_switch(GPID_PROCESS);   // setup virtual address space
    timer_reset();                     // start timer
    earth->intr_enable();              // enable interrupt
    sys_proc_entry(&pcb);              // enter sys_proc

    FATAL("Should never return to the grass kernel main()");
    return 0;
}

static int read_sys_proc(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

static void sys_proc_init() {
    INFO("Load kernel process #%d: sys_proc", GPID_PROCESS);
    elf_load(GPID_PROCESS, read_sys_proc, earth);

    pcb.proc_alloc = proc_alloc;
    pcb.proc_free = proc_free;
    pcb.proc_set_ready = proc_set_ready;
}
