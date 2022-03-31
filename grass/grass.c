/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block
 * and spawn the first kernel process, GPID_PROCESS
 */

#include "egos.h"
#include "grass.h"
#include <stdlib.h>

struct earth *earth = (void*)GRASS_STACK_TOP;
struct grass *grass = (void*)APPS_STACK_TOP;

static int sys_proc_read(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

int main() {
    SUCCESS("Enter the grass layer");

    proc_init();
    timer_init();
    INFO("Load kernel process #%d: sys_proc", GPID_PROCESS);
    elf_load(GPID_PROCESS, sys_proc_read, 0, NULL);

    grass->proc_alloc = proc_alloc;
    grass->proc_free = proc_free;
    grass->proc_set_ready = proc_set_ready;
    
    /* Enter the first kernel process sys_proc */
    earth->mmu_switch(GPID_PROCESS);
    timer_reset();
    earth->intr_enable();
    void (*sys_proc_entry)() = (void*)APPS_ENTRY;
    sys_proc_entry();

    FATAL("Should never return to the grass layer main()");
}
