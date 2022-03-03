/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: inode layer of the file system
 */

#include "app.h"
#include "file.h"
#include <string.h>

block_if fs;

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    /* Initialize the file system */
    block_if disk = fs_disk_init();    
    if (treedisk_create(disk, 0, NINODES) < 0)
        FATAL("proc_file: can't create treedisk file system");
    fs = treedisk_init(disk, 0);

    /* Send notification to GPID_PROCESS */
    int sender;
    char buf[SYSCALL_MSG_LEN];
    char* msg = "Finish GPID_FILE initialization";
    memcpy(buf, msg, 32);
    sys_send(GPID_PROCESS, buf, 32);

    /* Wait for file requests */
    while (1) {
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);
        struct file_request *req = (void*)buf;
        struct file_reply *reply = (void*)buf;

        int r, type = req->type;
        unsigned int ino = req->ino, offset = req->offset;
        switch (type) {
        case FILE_READ:
            r = fs->read(fs, ino, offset, (void*)&reply->block);
            reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            sys_send(sender, (void*)reply, sizeof(struct file_reply));
            break;
        case FILE_WRITE:
            FATAL("TODO: FILE_WRITE to be implemented in GPID_FILE");
            break;
        default:
            FATAL("GPID_FILE get unknown request %d", type);
        }
    }
    return 0;
}
