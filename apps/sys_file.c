/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: inode layer of the file system
 */

#include "app.h"
#include <string.h>

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    static int cnt = 0;
    char buf[30];
    char* msg = "Hi from GPID_FILE!";
    while (1) {
        if (cnt++ % 50000 == 0) {
            /* memcpy(buf, msg, 30); */
            sys_send(GPID_PROCESS, msg, sizeof(msg));
            sys_recv(buf, 30);
            HIGHLIGHT("In sys_file: received %s", buf);
        }
    }
    return 0;
}
