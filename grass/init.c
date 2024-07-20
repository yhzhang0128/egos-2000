/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: grass layer initialization
 * Spawn the first system server, GPID_PROCESS (pid=1).
 */

#include "egos.h"
#include "syscall.h"
#include "process.h"

static void sys_proc_read(uint block_no, char* dst) {
    earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

void grass_entry() {
    SUCCESS("Enter the grass layer");

    /* Initialize the grass interface functions */
    grass->proc_free = proc_free;
    grass->proc_alloc = proc_alloc;
    grass->proc_set_idle = proc_set_idle;
    grass->proc_set_ready = proc_set_ready;
    grass->proc_coresinfo = proc_coresinfo;

    grass->sys_exit = sys_exit;
    grass->sys_send = sys_send;
    grass->sys_recv = sys_recv;

    /* Initialize the IPC Buffer */
    msg_buffer->in_use = 0;

    /* Load the first system server GPID_PROCESS */
    INFO("Load kernel process #%d: sys_proc", GPID_PROCESS);
    elf_load(GPID_PROCESS, sys_proc_read, 0, 0);
    proc_set_running(proc_alloc());
    earth->mmu_switch(GPID_PROCESS);

    /* Finish using the kernel stack and thus release the lock */
    if (earth->platform == QEMU) release(earth->kernel_lock);

    /* Jump to the entry of process GPID_PROCESS */
    asm("mv a0, %0" ::"r"(APPS_ARG));
    asm("jr %0" :: "r" (APPS_ENTRY));
}
