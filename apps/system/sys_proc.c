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

static int app_ino, app_pid;
static void sys_spawn(int base);
static int app_spawn(struct proc_request *req);

int main() {
    SUCCESS("Enter kernel process GPID_PROCESS");    

    int sender, shell_waiting;
    char buf[SYSCALL_MSG_LEN];

    sys_spawn(SYS_FILE_EXEC_START);
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    INFO("sys_proc receives: %s", buf);

    sys_spawn(SYS_DIR_EXEC_START);
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    INFO("sys_proc receives: %s", buf);

    sys_spawn(SYS_SHELL_EXEC_START);
    
    while (1) {
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        struct proc_request *req = (void*)buf;
        struct proc_reply *reply = (void*)buf;
        if (req->type == PROC_KILLALL){
            grass->proc_free(-1);
        } else if (req->type == PROC_SPAWN) {
            reply->type = app_spawn(req) < 0 ? CMD_ERROR : CMD_OK;

            shell_waiting = (req->argv[req->argc - 1][0] != '&');
            if (!shell_waiting && app_pid > 0)
                INFO("process %d running in the background", app_pid);
            sys_send(GPID_SHELL, (void*)reply, sizeof(reply));
        } else if (req->type == PROC_EXIT) {
            grass->proc_free(sender);

            if (shell_waiting && app_pid == sender)
                sys_send(GPID_SHELL, (void*)reply, sizeof(reply));
            else
                INFO("background process %d terminated", sender);
        }
    }
}

static int app_read(int block_no, char* dst) {
    file_read(app_ino, block_no, dst);
}

static int app_spawn(struct proc_request *req) {
    int bin_ino = dir_lookup(0, "bin");
    if ((app_ino = dir_lookup(bin_ino, req->argv[0])) < 0) return -1;

    app_pid = grass->proc_alloc();
    int argc = req->argv[req->argc - 1][0] == '&'? req->argc - 1 : req->argc;

    elf_load(app_pid, app_read, argc, (void**)req->argv);
    grass->proc_set_ready(app_pid);
    return 0;
}

static int sys_proc_base;
static int sys_proc_read(int block_no, char* dst) {
    return earth->disk_read(sys_proc_base + block_no, 1, dst);
}

char* sysproc_names[] = {"sys_proc", "sys_file", "sys_dir", "sys_shell"};
static void sys_spawn(int base) {
    int pid = grass->proc_alloc();
    INFO("Load kernel process #%d: %s", pid, sysproc_names[pid - 1]);

    sys_proc_base = base;
    elf_load(pid, sys_proc_read, 0, NULL);
    grass->proc_set_ready(pid);
}
