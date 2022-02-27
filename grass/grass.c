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

    /* Initialization */
    proc_init();                       // process control block
    timer_init();                      // timer
    sys_proc_init();                   // first kernel process
    pcb.proc_alloc = proc_alloc;       // pcb interface to sys_proc
    pcb.proc_free = proc_free;
    pcb.proc_set_ready = proc_set_ready;

    /* Enter kernel process sys_proc */
    void (*sys_proc_entry)(void*) = (void*)VADDR_START;
    earth->mmu_switch(GPID_PROCESS);   // setup virtual address space
    timer_reset();                     // start timer
    earth->intr_enable();              // enable interrupt
    sys_proc_entry(&pcb);              // enter the application layer

    FATAL("Should never return to the grass kernel main()");
    return 0;
}

static int read_sys_proc(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

static void sys_proc_init() {
    INFO("Load kernel process #%d: process spawn and kill", GPID_PROCESS);
    struct block_store bs;
    bs.read = read_sys_proc;
    elf_load(GPID_PROCESS, &bs, earth);
}
