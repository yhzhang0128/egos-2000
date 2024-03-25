/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the file (inode) system server
 * handling requests to reading and writing inodes
 */

#include "app.h"
#include "file.h"
#include <string.h>

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    /* Initialize the file system interface */
    inode_intf fs = treedisk_init(fs_disk_init(), 0);

    /* Send a notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_FILE initialization");
    grass->sys_send(GPID_PROCESS, buf, 32);

    /* Wait for inode read/write requests */
    while (1) {
        int sender, r;
        struct file_request *req = (void*)buf;
        struct file_reply *reply = (void*)buf;
        grass->sys_recv(&sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case FILE_READ:
            r = fs->read(fs, req->ino, req->offset, (void*)&reply->block);
            reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case FILE_WRITE: default:
            /* This part is left to students as an exercise */
            FATAL("sys_file: request%d not implemented", req->type);
        }
    }
}
