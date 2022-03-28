/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple ls
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        INFO("ls with args not implemented");
        sys_exit(0);
    }

    int ino = grass->work_dir_ino;
    char* work_dir = grass->work_dir;

    int sender;
    struct file_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = FILE_READ;
    req.ino = ino;
    req.offset = 0;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);

    struct file_reply *reply = (void*)buf;
    char *result = reply->block.bytes;
    int len = strlen(result);

    for (int i = 1; i < len; i++)
        if (result[i - 1] == ' ' &&
            result[i] >= '0' &&
            result[i] <= '9')
            result[i] = ' ';

    printf("%s\r\n", result);

    return 0;
}
