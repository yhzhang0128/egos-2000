/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple shell
 */

#include "app.h"
#include <string.h>

#define tty_read earth->tty_read

#define KGRN  "\x1B[1;32m"
#define KCYN  "\x1B[1;36m"
#define KNRM  "\x1B[1;0m"

int get_inode(int ino, char* name);
void parse_request(char* buf, struct proc_request* req);

int main() {
    SUCCESS("Enter kernel process GPID_SHELL");

    int home = get_inode(0, "home");
    int yunhao = get_inode(home, "yunhao");

    grass->work_dir_ino = yunhao;
    char *work_dir = grass->work_dir;
    char *work_dir_name = grass->work_dir_name;
    strcpy(work_dir, "/home/yunhao");
    strcpy(work_dir_name, "yunhao");
    INFO("sys_shell: /home/yunhao has ino=%d", yunhao);
    
    /* Wait for shell commands */
    HIGHLIGHT("Welcome to egos-riscv!");
    char buf[128];
    while (1) {
        printf("%s➜ %s%s%s ", KGRN, KCYN, work_dir_name, KNRM);
        tty_read(buf, 100);
        if (strlen(buf) == 0) continue;

        if (strcmp(buf, "pwd") == 0) {
            printf("%s\r\n", work_dir);
        } else if (strncmp(buf, "cd", 2) == 0) {
            printf("sys_shell: `cd` not implemented; left to students\r\n");
        } else {
            int sender;
            struct proc_request req;
            struct proc_reply reply;
            parse_request(buf, &req);
            sys_send(GPID_PROCESS, (void*)&req, sizeof(req));

            if (req.argv[req.argc - 1][0] != '&') {
                /* Wait for the forground process */
                sys_recv(&sender, (void*)&reply, sizeof(reply));
                if (reply.type != CMD_OK)
                    INFO("sys_shell: command causes an error");
            }
        }
    }
    return 0;
}

int get_inode(int ino, char* name) {
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

void parse_request(char* buf, struct proc_request* req) {
    int len = strlen(buf);
    int idx = 0, nargs = 0;

    memset(req->argv, 0, CMD_NARGS * CMD_ARG_LEN);
    for (int i = 0; i < len; i++) {
        if (buf[i] != ' ') {
            req->argv[nargs][idx++] = buf[i];
        } else if (idx != 0) {
            nargs++;
            idx = 0;
        }
    }
    req->argc = ++nargs;
}
