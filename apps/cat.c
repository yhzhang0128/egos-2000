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
        INFO("please provide an argument");
        sys_exit(0);
    }

    int dir_ino = grass->work_dir_ino;
    char* file_name = argv[1];

    /* Get the inode number of the file */
    int sender;
    struct dir_request req1;
    char buf[SYSCALL_MSG_LEN];

    req1.type = DIR_LOOKUP;
    req1.ino = dir_ino;
    strcpy(req1.name, file_name);
    sys_send(GPID_DIR, (void*)&req1, sizeof(struct dir_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);

    struct dir_reply *reply1 = (void*)buf;
    if (reply1->status != DIR_OK) {
        INFO("file %s not found", file_name);
        sys_exit(1);
    }

    /* Read the file */
    struct file_request req2;

    req2.type = FILE_READ;
    req2.ino = reply1->ino;
    req2.offset = 0;
    sys_send(GPID_FILE, (void*)&req2, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);

    struct file_reply *reply2 = (void*)buf;
    char *result = reply2->block.bytes;
    int len = strlen(result);

    printf("%s", result);
    if (result[len - 1] != '\n')
        printf("\r\n");

    return 0;
}
