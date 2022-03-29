/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: directory layer of the file system
 */

#include "app.h"
#include <string.h>

int dir_do_lookup(int ino, char* name);

int main() {
    SUCCESS("Enter kernel process GPID_DIR");

    /* Send notification to GPID_PROCESS */
    int sender;
    char buf[SYSCALL_MSG_LEN];
    char* msg = "Finish GPID_DIR initialization";
    memcpy(buf, msg, 32);
    sys_send(GPID_PROCESS, buf, 32);

    /* Wait for dir requests */
    while (1) {
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);
        struct dir_request *req = (void*)buf;
        struct dir_reply *reply = (void*)buf;

        int type = req->type;
        int ino = req->ino;
        char* name = req->name;
        switch (type) {
        case DIR_LOOKUP:
            reply->ino = dir_do_lookup(ino, name);
            reply->status = reply->ino == -1? DIR_ERROR : DIR_OK;
            sys_send(sender, (void*)reply, sizeof(struct dir_reply));
            break;
        case DIR_INSERT:
        case DIR_REMOVE:
        default:
            FATAL("Request type=%d not implemented in GPID_DIR", type);
        }
    }
}

int dir_do_lookup(int ino, char* name) {
    char block[BLOCK_SIZE];
    file_read(ino, 0, block);

    int dir_len = strlen(block);
    int name_len = strlen(name);

    for (int i = 0; i < dir_len - name_len; i++) {
        int match = 1;
        for (int j = 0; j < name_len; j++)
            if (name[j] != block[i + j]) {
                match = 0;
                break;
            }

        if (match && block[i + name_len] == ' ') {
            int ino = 0, base = i + name_len;
            for (int k = 0; k < 4; k++) {
                char ch = block[base + k];
                if (ch != ' ')
                    ino = ino * 10 + ch - '0';
            }
            return ino;
        }
    }

    return -1;
}
