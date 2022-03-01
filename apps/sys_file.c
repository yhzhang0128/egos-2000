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

block_store_t disk;

int disk_getsize(block_store_t *this_bs, unsigned int ino){
    return FS_DISK_SIZE / BLOCK_SIZE;
}

int disk_setsize(block_store_t *this_bs, unsigned int ino, block_no newsize) {
    FATAL("disk_setsize not implemented");
    return -1;
}

int disk_read(block_store_t *this_bs, unsigned int ino, block_no offset, block_t *block) {
    return earth->disk_read(GRASS_FS_START + offset, 1, block->bytes);
}

int disk_write(block_store_t *this_bs, unsigned int ino, block_no offset, block_t *block) {
    return earth->disk_write(GRASS_FS_START + offset, 1, block->bytes);
}


int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    disk.read = disk_read;
    disk.write = disk_write;
    disk.getsize = disk_getsize;
    disk.setsize = disk_setsize;

    if (treedisk_create(&disk, 0, NINODES) < 0)
        FATAL("proc_file: can't create treedisk file system");
    block_store_t *bs = treedisk_init(&disk, 0);

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
