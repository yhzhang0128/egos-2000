/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: directory layer of the file system
 */

#include "app.h"
#include <string.h>

int dir_lookup(int ino, char* name);

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
            reply->ino = dir_lookup(ino, name);
            reply->status = reply->ino == -1? DIR_ERROR : DIR_OK;
            sys_send(sender, (void*)reply, sizeof(struct dir_reply));
            break;
        case DIR_INSERT:
            FATAL("TODO: DIR_INSERT to be implemented in GPID_DIR");
            break;
        case DIR_REMOVE:
            FATAL("TODO: DIR_REMOVE to be implemented in GPID_DIR");
            break;
        default:
            FATAL("GPID_DIR get unknown request %d", type);
        }
    }
    return 0;
}

int dir_lookup(int ino, char* name) {
    int len = strlen(name);
    int sender;
    struct file_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = FILE_READ;
    req.ino = ino;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("sys_dir expects message from GPID_FILE");

    struct file_reply *reply = (void*)buf;
    int name_len = strlen(name);
    int dir_len = strlen(reply->block.bytes);

    if (dir_len < name_len)
        return -1;

    for (int i = 0; i < dir_len - name_len; i++) {
        int match = 1;
        for (int j = 0; j < name_len; j++)
            if (name[j] != reply->block.bytes[i + j]) {
                match = 0;
                break;
            }
        if (match) {
            int ino = 0, base = i + name_len;
            for (int k = 0; k < 4; k++) {
                char ch = reply->block.bytes[base + k];
                if (ch != ' ')
                    ino = ino * 10 + ch - '0';
            }
            return ino;
        }
    }

    return -1;
}
