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

static int app_ino, app_pid;
static void sys_spawn(int, int);
static int app_spawn(struct proc_request *req);

int main(struct pcb_intf* _pcb) {
    SUCCESS("Enter kernel process GPID_PROCESS");    
    memcpy(&pcb, _pcb, sizeof(struct pcb_intf));

    int sender, shell_waiting;
    char buf[SYSCALL_MSG_LEN];
    sys_spawn(GPID_FILE, SYS_FILE_EXEC_START);
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    INFO("sys_proc receives: %s", buf);

    sys_spawn(GPID_DIR, SYS_DIR_EXEC_START);
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    INFO("sys_proc receives: %s", buf);

    sys_spawn(GPID_SHELL, SYS_SHELL_EXEC_START);
    
    while (1) {
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        struct proc_request *req = (void*)buf;
        struct proc_reply *reply = (void*)buf;
        if (req->type == PROC_SPAWN) {
            reply->type = app_spawn(req) < 0 ? CMD_ERROR : CMD_OK;
            shell_waiting = (req->argv[req->argc - 1][0] != '&');
            if (!shell_waiting)
                INFO("process %d running in the background", app_pid);
            sys_send(GPID_SHELL, (void*)reply, sizeof(reply));
        } else if (req->type == PROC_EXIT) {
            pcb.proc_free(sender);
            if (shell_waiting && app_pid == sender) {
                sys_send(GPID_SHELL, (void*)reply, sizeof(reply));
            } else {
                INFO("background process %d terminated", sender);
            }
                
        } else if (req->type == PROC_KILLALL){
            pcb.proc_free(-1);
        } else {
            FATAL("sys_proc: unexpected message type %d", req->type);
        }

    }
}

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

static int app_spawn(struct proc_request *req) {
    int bin_ino = get_inode(0, "bin");
    app_ino = get_inode(bin_ino, req->argv[0]);

    if (app_ino < 0) {
        return -1;
    } else {
        app_pid = pcb.proc_alloc();
        if (req->argv[req->argc - 1][0] != '&') 
            elf_load(app_pid, app_read, req->argc, (void**)req->argv);
        else
            elf_load(app_pid, app_read, req->argc - 1, (void**)req->argv);
        pcb.proc_set_ready(app_pid);
        return 0;
    }
}

static int SYS_PROC_BASE;
static int sys_proc_read(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_BASE + block_no, 1, dst);
}

char* kernel_procs[] = {"", "sys_proc", "sys_file", "sys_dir", "sys_shell"};
static void sys_spawn(int pid, int base) {
    int _pid = pcb.proc_alloc();
    if (_pid != pid)
        FATAL("Process ID mismatch: %d != %d", pid, _pid);

    INFO("Load kernel process #%d: %s", pid, kernel_procs[pid]);
    SYS_PROC_BASE = base;
    elf_load(pid, sys_proc_read, 0, NULL);
    pcb.proc_set_ready(pid);
}
