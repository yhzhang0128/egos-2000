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

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    /* Initialize the file system */
    block_if disk = fs_disk_init();    
    if (treedisk_create(disk, 0, NINODES) < 0)
        FATAL("proc_file: can't create treedisk file system");
    block_if fs = treedisk_init(disk, 0);

    /* Send notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_FILE initialization");
    sys_send(GPID_PROCESS, buf, strlen(buf) + 1);

    /* Wait for file requests */
    while (1) {
        int sender, r;
        struct file_request *req = (void*)buf;
        struct file_reply *reply = (void*)buf;
        sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case FILE_READ:
            r = fs->read(fs, req->ino, req->offset, (void*)&reply->block);
            reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case FILE_WRITE:
        default:
            FATAL("proc_file: request%d not implemented", req->type);
        }
    }
}
