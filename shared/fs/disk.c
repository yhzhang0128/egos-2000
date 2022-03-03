#include "egos.h"
#include "disk.h"
#include "fs.h"
#include "print.h"

static int disk_getsize(block_if this_bs, unsigned int ino){
    return FS_DISK_SIZE / BLOCK_SIZE;
}

static int disk_setsize(block_if this_bs, unsigned int ino, block_no newsize) {
    FATAL("Cannot set the size of the physical disk");
    return -1;
}

static int disk_read(block_if this_bs, unsigned int ino, block_no offset, block_t *block) {
    return earth->disk_read(GRASS_FS_START + offset, 1, block->bytes);
}

static int disk_write(block_if this_bs, unsigned int ino, block_no offset, block_t *block) {
    return earth->disk_write(GRASS_FS_START + offset, 1, block->bytes);
}

static block_store_t disk;

block_if fs_disk_init() {
    disk.read = disk_read;
    disk.write = disk_write;
    disk.getsize = disk_getsize;
    disk.setsize = disk_setsize;

    return &disk;
}
