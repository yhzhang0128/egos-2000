/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: directory layer of the file system
 */

#include "app.h"
#include "fs.h"
#include <string.h>

#define CACHE_NDIR 128
struct dir_entry {
    int  ino;
    char name[DIR_NAME_SIZE];
} cache[CACHE_NDIR];
void dir_init();

int main() {
    SUCCESS("Enter kernel process GPID_DIR");

    /* Initialize the directory table */
    dir_init();

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

        int r, type = req->type;
        char* name = req->name;
        unsigned int ino = req->ino;
        switch (type) {
        case DIR_LOOKUP:
            FATAL("TODO: DIR_LOOKUP to be implemented in GPID_DIR");
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


void dir_init() {
    int sender;
    struct file_request req;
    char buf[SYSCALL_MSG_LEN];
    req.type = FILE_READ;
    req.ino = 0;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("sys_dir expects message from GPID_FILE");
    HIGHLIGHT("Get dir table:");

    struct file_reply *reply = (void*)buf;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        char ch = reply->block.bytes[i];
        switch (ch) {
        case 0:
            i = BLOCK_SIZE;
            break;
        case '\n':
            printf("\r\n");
            break;
        default:
            printf("%c", ch);
        }
    }
}
