/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple cat
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        INFO("cat: please provide an argument");
        return 0;
    }

    /* Get the inode number of the file */
    int file_ino = dir_lookup(grass->work_dir_ino, argv[1]);
    if (file_ino < 0) {
        INFO("cat: file %s not found", argv[1]);
        return 1;
    }

    /* Read the first block of the file */
    struct file_request req;
    req.type = FILE_READ;
    req.ino = file_ino;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(req));

    int sender;
    char buf[SYSCALL_MSG_LEN];
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    struct file_reply *reply = (void*)buf;
    char *result = reply->block.bytes;

    printf("%s", result);
    if (result[strlen(result) - 1] != '\n')
        printf("\r\n");

    return 0;
}
