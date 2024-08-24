/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: the file system server
 * Manage the disk device; Handle file (inode) read and write for other apps
 */

#include "app.h"
#include "inode.h"
#include <string.h>

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    /* Initialize the file system interface */
    inode_intf fs = (FILESYS == 0)? mydisk_init(fs_disk_init(), 0) : treedisk_init(fs_disk_init(), 0);

    /* Send a notification to GPID_PROCESS */
    char buf[SYSCALL_MSG_LEN];
    strcpy(buf, "Finish GPID_FILE initialization");
    grass->sys_send(GPID_PROCESS, buf, 32);

    /* Wait for inode read/write requests */
    while (1) {
        int sender, r;
        struct file_request* req = (void*)buf;
        struct file_reply* reply = (void*)buf;
        grass->sys_recv(GPID_ALL, &sender, buf, SYSCALL_MSG_LEN);

        switch (req->type) {
        case FILE_READ:
            r = fs->read(fs, req->ino, req->offset, (void*)&reply->block);
            reply->status = r == 0 ? FILE_OK : FILE_ERROR;
            grass->sys_send(sender, (void*)reply, sizeof(*reply));
            break;
        case FILE_WRITE:
        default:
            /* This part is left to students as an exercise */
            FATAL("sys_file: request%d not implemented", req->type);
        }
    }
}
