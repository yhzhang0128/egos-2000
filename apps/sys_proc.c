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

    sys_shell_init();
    
    while (1) {
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        struct proc_request *req = (void*)buf;
        if (sender == GPID_SHELL) {
            INFO("sys_proc: got shell request with %d args", req->argc);
            for (int i = 0; i < req->argc; i++)
                INFO("%s", req->argv[i]);

            struct proc_reply *reply = (void*)buf;
            reply->type = CMD_OK;
            sys_send(GPID_SHELL, (void*)reply, sizeof(struct proc_reply));
        } else {
            // process killed
        }

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

static int sys_shell_read(int block_no, char* dst) {
    return earth->disk_read(SYS_SHELL_EXEC_START + block_no, 1, dst);
}

void sys_shell_init() {
    int shell_pid = pcb.proc_alloc();
    if (shell_pid != GPID_SHELL)
        FATAL("Process ID mismatch: %d != %d", shell_pid, GPID_SHELL);

    INFO("Load kernel process #%d: sys_shell", shell_pid);
    elf_load(shell_pid, sys_shell_read, earth);
    pcb.proc_set_ready(shell_pid);
}
