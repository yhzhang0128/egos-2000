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
static int proc_spawn(struct proc_request *req);


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
            if (proc_spawn(req) != 0) {
                struct proc_reply *reply = (void*)buf;
                reply->type = CMD_ERROR;
                sys_send(GPID_SHELL, (void*)reply, sizeof(struct proc_reply));
            }
        } else if (req->type == PROC_KILLED) {
            struct proc_reply *reply = (void*)buf;
            reply->pid = sender;
            reply->type = CMD_OK;
            sys_send(GPID_SHELL, (void*)reply, sizeof(struct proc_reply));
        } else {
            FATAL("sys_proc: receive unexpected message");
        }

    }
}


static int app_ino;
static int app_read(int block_no, char* dst) {
    int sender;
    struct file_request req;
    char buf[SYSCALL_MSG_LEN];
    req.type = FILE_READ;
    req.ino = app_ino;
    req.offset = block_no;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("app_read expects message from GPID_FILE");

    struct file_reply *reply = (void*)buf;
    memcpy(dst, reply->block.bytes, BLOCK_SIZE);
    return 0;
}

void app_init(struct proc_request *req) {
    int app_pid = pcb.proc_alloc();
    elf_load_with_arg(app_pid, app_read, req->argc, (void**)req->argv, earth);
    pcb.proc_set_ready(app_pid);
}


static int get_inode(int ino, char* name);
static int proc_spawn(struct proc_request *req) {
    int bin = get_inode(0, "bin");
    int exec = get_inode(bin, req->argv[0]);

    if (exec == -1) {
        return -1;
    } else {
        INFO("sys_proc: spawn the process");
        app_ino = exec;
        app_init(req);
        return 0;
    }
}

static int get_inode(int ino, char* name) {
    int sender;
    struct dir_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = DIR_LOOKUP;
    req.ino = ino;
    strcpy(req.name, name);
    sys_send(GPID_DIR, (void*)&req, sizeof(struct dir_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_DIR)
        FATAL("sys_shell expects message from GPID_DIR");

    struct dir_reply *reply = (void*)buf;
    return reply->ino;
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
