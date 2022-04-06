/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: directory layer of the file system
 */

#include "app.h"
#include <string.h>

int dir_do_lookup(int dir_ino, char* name) {
    char block[BLOCK_SIZE];
    file_read(dir_ino, 0, block);

    int dir_len = strlen(block);
    int name_len = strlen(name);

    for (int i = 0, ret = 0; i < dir_len - name_len; i++)
        if (strncmp(name, block + i, name_len) == 0 &&
            block[i + name_len] == ' ') {
            for (int k = 0; k < 4; k++)
                if (block[i + name_len + k] != ' ')
                    ret = ret * 10 + block[i + name_len + k] - '0';
            return ret;
        }

    return -1;
}

int main() {
    SUCCESS("Enter kernel process GPID_DIR");

    /* Send notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_DIR initialization");
    sys_send(GPID_PROCESS, buf, strlen(buf) + 1);

    /* Wait for dir requests */
    while (1) {
        int sender;
        struct dir_request *req = (void*)buf;
        struct dir_reply *reply = (void*)buf;
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case DIR_LOOKUP:
            reply->ino = dir_do_lookup(req->ino, req->name);
            reply->status = reply->ino == -1? DIR_ERROR : DIR_OK;
            sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case DIR_INSERT:
        case DIR_REMOVE:
        default:
            FATAL("proc_dir: request%d not implemented", req->type);
        }
    }
}
