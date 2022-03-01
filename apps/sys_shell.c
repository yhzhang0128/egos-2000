/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple shell
 */

#include "app.h"
#include "fs.h"
#include <string.h>

int get_inode(int ino, char* name);

int main() {
    SUCCESS("Enter kernel process GPID_SHELL");

    // get the inode of /home/yunhao
    char work_dir[256];
    int home = get_inode(0, "home");
    int yunhao = get_inode(home, "yunhao");
    strcpy(work_dir, "/home/yunhao");
    HIGHLIGHT("sys_shell: /home/yunhao has ino=%d", yunhao);

    /* Wait for shell commands */
    while (1) {
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
