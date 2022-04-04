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

    char work_dir[] = "/home/yunhao";
    strcpy(grass->work_dir_name, "yunhao");

    int home_ino = dir_lookup(0, "home");
    grass->work_dir_ino = dir_lookup(home_ino, "yunhao");
    INFO("sys_shell: %s has ino=%d", work_dir, grass->work_dir_ino);
    CRITICAL("Welcome to egos-2000!");
    
    /* Wait for shell commands */
    while (1) {
        printf("%s➜ %s%s%s ", "\x1B[1;32m", "\x1B[1;36m", grass->work_dir_name, "\x1B[1;0m");
        char buf[256];
        if (earth->tty_read(buf, 256) == 0) continue;

        struct proc_request req;
        struct proc_reply reply;

        if (strcmp(buf, "pwd") == 0) {
            printf("%s\r\n", work_dir);
        } else if (strcmp(buf, "clear") == 0) {
            printf("\e[1;1H\e[2J");
        } else if (strcmp(buf, "killall") == 0) {
            req.type = PROC_KILLALL;
            sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
        } else {
            int sender;
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
