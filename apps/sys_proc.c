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
void sys_dir_init();
void sys_shell_init();

int main(struct pcb_intf* _pcb) {
    SUCCESS("Enter kernel process GPID_PROCESS");    
    memcpy(&pcb, _pcb, sizeof(struct pcb_intf));

    int sender;
    char buf[SYSCALL_MSG_LEN];
    sys_file_init();
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("sys_proc expects message from GPID_FILE");
    INFO("sys_proc receives message: %s", buf);


    struct file_request req;
    req.type = FILE_READ;
    req.ino = 0;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("sys_proc expects message from GPID_FILE");
    HIGHLIGHT("GPID_PROCESS Get dir table:");

    struct file_reply *reply = (void*)buf;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        char ch = reply->block.bytes[i];
        switch (ch) {
        case 0:
            i = BLOCK_SIZE;
            break;
        case '\n':
            printf("\r\n");
            break;
        default:
            printf("%c", ch);
        }
    }
    
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
