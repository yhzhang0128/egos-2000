/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: inode layer of the file system
 */

#include "app.h"
#include "fs.h"
#include <string.h>

block_if treedisk;
void dirtable_dump();

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    block_if disk = fs_disk_init();    
    if (treedisk_create(disk, 0, NINODES) < 0)
        FATAL("proc_file: can't create treedisk file system");
    treedisk = treedisk_init(disk, 0);
    dirtable_dump();

    char buf[SYSCALL_MSG_LEN];
    char* msg = "Finish GPID_FILE initialization";
    memcpy(buf, msg, 32);
    sys_send(GPID_PROCESS, buf, 32);

    while (1) {
        sys_recv(buf, SYSCALL_MSG_LEN);
        struct file_request *req = (void*)buf;
        INFO("GPID_FILE get request type=%d ino=%d offset=%d", req->type, req->ino, req->offset);
    }
    return 0;
}


void dirtable_dump() {
    char buf[BLOCK_SIZE];
    treedisk->read(treedisk, 0, 0, (void*)buf);
    INFO("Get dir table:");

    for (int i = 0; i < BLOCK_SIZE; i++) {
        switch (buf[i]) {
        case 0:
            i = BLOCK_SIZE;
            break;
        case '\n':
            printf("\r\n");
            break;
        default:
            printf("%c", buf[i]);
        }
    }
}
