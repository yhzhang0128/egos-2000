/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: the system server for spawning and killing processes
 */

#include "app.h"
#include "elf.h"
#include "disk.h"
#include <string.h>

static int app_ino, app_pid;
static void sys_spawn(uint base);
static int app_spawn(struct proc_request *req);

int main() {
    SUCCESS("Enter kernel process GPID_PROCESS");
    
    int sender, shell_waiting;
    char buf[SYSCALL_MSG_LEN];

    sys_spawn(SYS_TERM_EXEC_START);
    grass->sys_recv(GPID_TERMINAL, NULL, buf, SYSCALL_MSG_LEN);
    INFO("sys_process receives: %s", buf);

    sys_spawn(SYS_FILE_EXEC_START);
    grass->sys_recv(GPID_FILE, NULL, buf, SYSCALL_MSG_LEN);
    INFO("sys_process receives: %s", buf);

    sys_spawn(SYS_SHELL_EXEC_START);

    while (1) {
        struct proc_request* req = (void*)buf;
        struct proc_reply* reply = (void*)buf;
        grass->sys_recv(GPID_ALL, &sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case PROC_SPAWN:
            reply->type = app_spawn(req);

            shell_waiting = (req->argv[req->argc - 1][0] != '&') && (reply->type == CMD_OK);
            if (!shell_waiting && reply->type == CMD_OK)
                INFO("process %d running in the background", app_pid);
            grass->sys_send(GPID_SHELL, (void*)reply, sizeof(*reply));
            break;
        case PROC_EXIT:
            grass->proc_free(sender);

            if (shell_waiting && app_pid == sender)
                grass->sys_send(GPID_SHELL, (void*)reply, sizeof(*reply));
            else if (app_pid == sender)
                INFO("background process %d terminated", sender);
            break;
        case PROC_KILLALL:
            grass->proc_free(GPID_ALL);
            break;
        default:
            FATAL("sys_process: invalid request %d", req->type);
        }
    }
}

static void app_read(uint off, char* dst) {
    file_read(app_ino, off, dst);
}

static int app_spawn(struct proc_request* req) {
    int bin_ino = dir_lookup(0, "bin/");
    if ((app_ino = dir_lookup(bin_ino, req->argv[0])) < 0) return CMD_ERROR;
    int argc = req->argv[req->argc - 1][0] == '&'? req->argc - 1 : req->argc;

    app_pid = grass->proc_alloc();
    elf_load(app_pid, app_read, argc, (void**)req->argv);
    grass->proc_set_ready(app_pid);

    return CMD_OK;
}

static int sys_proc_base;
char* sysproc_names[] = {"sys_process", "sys_terminal", "sys_file", "sys_shell"};

static void sys_proc_read(uint block_no, char* dst) {
    earth->disk_read(sys_proc_base + block_no, 1, dst);
}

static void sys_spawn(uint base) {
    int pid = grass->proc_alloc();
    INFO("Load kernel process #%d: %s", pid, sysproc_names[pid - 1]);

    sys_proc_base = base;
    elf_load(pid, sys_proc_read, 0, NULL);
    grass->proc_set_ready(pid);
}
