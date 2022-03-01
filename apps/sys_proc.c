/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: spawn and kill processes
 */


#include "elf.h"
#include "app.h"
#include "disk.h"
#include <string.h>

/* same as the struct pcb_intf in grass/process.h */
struct pcb_intf {
    int (*proc_alloc)();
    void (*proc_free)(int);
    void (*proc_set_ready)(int);
} pcb;

void sys_file_init();


int main(struct pcb_intf* _pcb) {
    SUCCESS("Enter kernel process GPID_PROCESS");    
    memcpy(&pcb, _pcb, sizeof(struct pcb_intf));

    sys_file_init();
    char buf[SYSCALL_MSG_LEN];
    sys_recv(buf, SYSCALL_MSG_LEN);
    INFO("sys_proc receives message: %s", buf);
    

    struct file_request req;
    req.type = FILE_READ;
    req.ino = 0;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));

    while (1) {
    }
}

static int sys_file_read(int block_no, char* dst) {
    return earth->disk_read(SYS_FILE_EXEC_START + block_no, 1, dst);
}

void sys_file_init() {
    int file_pid = pcb.proc_alloc();
    if (file_pid != GPID_FILE)
        FATAL("Process ID mismatch: %d != %d", file_pid, GPID_FILE);

    INFO("Load kernel process #%d: sys_file", file_pid);
    elf_load(file_pid, sys_file_read, earth);
    pcb.proc_set_ready(file_pid);
}
