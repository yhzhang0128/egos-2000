/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple shell
 */

#include "app.h"
#include <string.h>

int parse_request(char* buf, struct proc_request* req) {
    int idx = 0, nargs = 0;
    memset(req->argv, 0, CMD_NARGS * CMD_ARG_LEN);

    for (int i = 0; i < strlen(buf); i++)
        if (buf[i] != ' ') {
            req->argv[nargs][idx] = buf[i];
            if (++idx >= CMD_ARG_LEN) return -1;
        } else if (idx != 0) {
            idx = req->argv[nargs][idx] = 0;
            if (++nargs >= CMD_NARGS) return -1;
        }
    req->argc = idx ? nargs + 1 : nargs;
    return 0;
}

int main() {
    CRITICAL("Welcome to the egos-2000 shell!");
    
    char buf[256] = "cd";  /* Enter the home directory first */
    while (1) {
        struct proc_request req;
        struct proc_reply reply;

        if (strcmp(buf, "killall") == 0) {
            req.type = PROC_KILLALL;
            grass->sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
        } else {
            req.type = PROC_SPAWN;

            if (0 != parse_request(buf, &req)) {
                INFO("sys_shell: too many arguments or argument too long");
            } else {
                grass->sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
                grass->sys_recv(NULL, (void*)&reply, sizeof(reply));

                if (reply.type != CMD_OK)
                    INFO("sys_shell: command causes an error");
                else if (req.argv[req.argc - 1][0] != '&')
                    /* Wait for foreground command to terminate */
                    grass->sys_recv(NULL, (void*)&reply, sizeof(reply));
            }
        }

        do {
            printf("\x1B[1;32m➜ \x1B[1;36m%s\x1B[1;0m ", grass->workdir);
        } while (earth->tty_read(buf, 256) == 0);
    }
}
