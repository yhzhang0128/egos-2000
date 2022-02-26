/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: spawn and kill processes
 */


#include "app.h"
#include <string.h>

/* same as the struct pcb_intf in grass/process.h */
struct pcb_intf {
    long long (*timer_reset)();
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_running)(int);
    void (*proc_set_runnable)(int);
} pcb;

int main(struct pcb_intf* _pcb) {
    SUCCESS("Enter kernel process GPID_PROCESS");    
    memcpy(&pcb, _pcb, sizeof(struct pcb_intf));

    int file_pid = pcb.proc_alloc();
    if (file_pid != GPID_FILE)
        FATAL("Process ID mismatch: %d != %d", file_pid, GPID_FILE);
    INFO("Load kernel process #%d: file server", file_pid);


    earth->intr_enable();
    pcb.timer_reset();

    while (1);
}
