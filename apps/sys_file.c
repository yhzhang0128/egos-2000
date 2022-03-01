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

    block_if disk = disk_init();    
    if (treedisk_create(disk, 0, NINODES) < 0)
        FATAL("proc_file: can't create treedisk file system");
    treedisk = treedisk_init(disk, 0);
    dirtable_dump();

    static int cnt = 0;
    char buf[30];
    char* msg = "Hi from GPID_FILE!";
    while (1) {
        if (cnt++ % 50000 == 0) {
            memcpy(buf, msg, 30);
            sys_send(GPID_PROCESS, buf, 30);
            sys_recv(buf, 30);
            HIGHLIGHT("In sys_file: received %s", buf);
        }
    }
    return 0;
}


void dirtable_dump() {
    char buf[BLOCK_SIZE];
    treedisk->read(treedisk, 0, 0, (void*)buf);
    HIGHLIGHT("Get dir table:");

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
