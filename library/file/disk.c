/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: functions for accessing the physical disk
 */

#include "egos.h"
#include "inode.h"

static int disk_getsize() { return FILE_SYS_DISK_SIZE / BLOCK_SIZE; }

static int disk_setsize() { FATAL("disk: cannot set size"); }

static int disk_read(inode_intf bs, uint ino, block_no offset, block_t* block) {
    earth->disk_read(FILE_SYS_DISK_START + offset, 1, block->bytes);
    return 0;
}

static int disk_write(inode_intf bs, uint ino, block_no offset, block_t* block) {
    earth->disk_write(FILE_SYS_DISK_START + offset, 1, block->bytes);
    return 0;
}

static inode_store_t disk;

inode_intf fs_disk_init() {
    disk.read    = disk_read;
    disk.write   = disk_write;
    disk.getsize = disk_getsize;
    disk.setsize = disk_setsize;

    return &disk;
}
