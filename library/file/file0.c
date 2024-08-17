/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a dummy file system illustrating the concept of "inode"
 */

#include "egos.h"
#include "inode.h"
#include <string.h>
#include <stdlib.h>

#define DUMMY_INO_MAX_SIZE       128
#define DUMMY_INO_DISK_OFFSET(x) x * DUMMY_INO_MAX_SIZE

static int mydisk_getsize(inode_store_t* this_bs, uint ino) {
    inode_intf below = this_bs->state;

    block_t ino_size_block;
    (below->read)(below, 0, DUMMY_INO_DISK_OFFSET(ino), &ino_size_block);
    return *((int*)ino_size_block.bytes);
}

static int mydisk_setsize(inode_store_t* this_bs, uint ino, block_no nblocks){
    return -1;
}

static int mydisk_read(inode_store_t* this_bs, uint ino, block_no offset, block_t* block){
    inode_intf below = this_bs->state;
    return (below->read)(below, 0, DUMMY_INO_DISK_OFFSET(ino) + offset + 1, block);
}

static int mydisk_write(inode_store_t* this_bs, uint ino, block_no offset, block_t* block){
    inode_intf below = this_bs->state;
    uint disk_offset = DUMMY_INO_DISK_OFFSET(ino);

    if (mydisk_getsize(this_bs, ino) <= offset) {
        /* Update inode size to offset + 1 */
        block_t ino_size_block;
        int* new_size = (int*)ino_size_block.bytes;
        *new_size     = offset + 1;
        (below->write)(below, 0, disk_offset, &ino_size_block);
    }

    return (below->write)(below, 0, disk_offset + offset + 1, block);
}

inode_intf mydisk_init(inode_intf below, uint below_ino) {
    /* This dummy file system assumes that below_ino == 0 */
    if (below_ino != 0) return NULL;

    inode_store_t* this_bs = malloc(sizeof(inode_store_t));
    memset(this_bs, 0, sizeof(inode_store_t));
    this_bs->getsize = mydisk_getsize;
    this_bs->setsize = mydisk_setsize;
    this_bs->read    = mydisk_read;
    this_bs->write   = mydisk_write;
    this_bs->state   = below;
    return this_bs;
}

int mydisk_create(inode_intf below, uint below_ino, uint ninodes) {
    return 0;
}
