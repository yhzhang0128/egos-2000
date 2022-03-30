/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple shell
 */

#include "app.h"
#include <string.h>

void parse_request(char* buf, struct proc_request* req) {
    int idx = 0, nargs = 0;
    memset(req->argv, 0, CMD_NARGS * CMD_ARG_LEN);

    for (int i = 0; i < strlen(buf); i++)
        if (buf[i] != ' ') {
            req->argv[nargs][idx++] = buf[i];
        } else if (idx != 0) {
            nargs++;
            idx = 0;
        }
    req->argc = idx ? nargs + 1 : nargs;
}

int main() {
    SUCCESS("Enter kernel process GPID_SHELL");

    strcpy(grass->work_dir, "/home/yunhao");
    strcpy(grass->work_dir_name, "yunhao");

    int home = dir_lookup(0, "home");
    grass->work_dir_ino = dir_lookup(home, "yunhao");
    INFO("sys_shell: /home/yunhao has ino=%d", grass->work_dir_ino);
    HIGHLIGHT("Welcome to egos-riscv!");
    
    /* Wait for shell commands */
    int sender;
    char buf[256];
    struct proc_request req;
    struct proc_reply reply;
    while (1) {
        printf("%s➜ %s%s%s ", "\x1B[1;32m", "\x1B[1;36m", grass->work_dir_name, "\x1B[1;0m");
        earth->tty_read(buf, 256);
        if (strlen(buf) == 0) continue;

        if (strcmp(buf, "pwd") == 0) {
            printf("%s\r\n", grass->work_dir);
        } else if (strcmp(buf, "clear") == 0) {
            printf("\e[1;1H\e[2J");
        } else if (strcmp(buf, "killall") == 0) {
            req.type = PROC_KILLALL;
            sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
        } else {
            req.type = PROC_SPAWN;
            parse_request(buf, &req);
            sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
            sys_recv(&sender, (void*)&reply, sizeof(reply));

            if (reply.type != CMD_OK)
                INFO("sys_shell: command causes an error");
            else if (req.argv[req.argc - 1][0] != '&')
                /* Wait for foreground process */
                sys_recv(&sender, (void*)&reply, sizeof(reply));            
        }
    }
}
