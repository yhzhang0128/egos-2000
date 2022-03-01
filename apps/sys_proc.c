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
    INFO("sys_proc receives: %s", buf);

    sys_dir_init();
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_DIR)
        FATAL("sys_proc expects message from GPID_DIR");
    INFO("sys_proc receives: %s", buf);

    struct dir_request req;
    req.type = DIR_LOOKUP;
    req.ino = 0;
    req.name[0] = 'h';
    req.name[1] = 'o';
    req.name[2] = 'm';
    req.name[3] = 'e';
    req.name[4] = 0;
    sys_send(GPID_DIR, (void*)&req, sizeof(struct dir_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    struct dir_reply *reply = (void*)buf;
    HIGHLIGHT("sys_proc: ino=%d for home", reply->ino);    
    
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

static int sys_dir_read(int block_no, char* dst) {
    return earth->disk_read(SYS_DIR_EXEC_START + block_no, 1, dst);
}

void sys_dir_init() {
    int dir_pid = pcb.proc_alloc();
    if (dir_pid != GPID_DIR)
        FATAL("Process ID mismatch: %d != %d", dir_pid, GPID_DIR);

    INFO("Load kernel process #%d: sys_dir", dir_pid);
    elf_load(dir_pid, sys_dir_read, earth);
    pcb.proc_set_ready(dir_pid);
}
