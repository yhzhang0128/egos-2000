/*
 * (C) 2017, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse, August 2015
 *
 * This is the include file for all inode store modules.  An inode store
 * maintains one or more "inodes", each representing an array of blocks.
 * A physical disk would typically just have one inode (inode 0), while
 * a virtualized disk may have many.  Each inode store module has an
 * 'init' function that returns a inode_store_t *.  The inode_store_t * is
 * a pointer to a structure that contains the following seven methods:
 *
 *      int getsize(inode_store_t *this_bs, unsigned int ino)
 *          returns the size of the inode store at the given inode number
 *			(inode numbers start at 0)
 *
 *      int setsize(inode_store_t *this_bs, unsigned int ino, block_no newsize)
 *          set the size of the inode store at the given inode number
 *          returns the old size
 *
 *      int read(inode_store_t *this_bs, unsigned int ino, block_no offset, block_t *block)
 *          read the block at the given inode number and offset and return in *block
 *          returns 0
 *
 *      int write(inode_store_t *this_bs, unsigned int ino, block_no offset, block_t *block)
 *          write *block to the block at the given inode number and offset
 *          returns 0
 *
 * All these return -1 upon error (typically after printing the
 * reason for the error).
 *
 * An inode_store_t * also maintains a void* pointer called 'state' to internal
 * state the inode store module needs to keep.
 */

#pragma once
#include "disk.h"

#define NINODES  128

typedef struct inode_store {
    int (*getsize)(struct inode_store *this_bs, unsigned int ino);
    int (*setsize)(struct inode_store *this_bs, unsigned int ino, block_no newsize);
    int (*read)(struct inode_store *this_bs, unsigned int ino, block_no offset, block_t *block);
    int (*write)(struct inode_store *this_bs, unsigned int ino, block_no offset, block_t *block);
    void *state;
} inode_store_t;

typedef inode_store_t *inode_intf;    /* inode store interface */

inode_intf fs_disk_init();
inode_intf treedisk_init(inode_intf below, unsigned int below_ino);
int treedisk_create(inode_intf below, unsigned int below_ino, unsigned int ninodes);