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
 * 'init' function that returns a inode_intf.  The inode_intf is
 * a pointer to a structure that contains the following four methods:
 *
 * int getsize(inode_intf self, unsigned int ino)
 *   - returns the size of the inode store at the given inode number
 *     (inode numbers start at 0)
 *
 * int setsize(inode_intf self, unsigned int ino, uint newsize)
 *   - sets the size of the inode store at the given inode number
 *     returns the old size
 *
 * int read(inode_intf self, unsigned int ino, uint offset, block_t *block)
 *   - reads the block at the given inode number and offset and return in *block
 *
 * int write(inode_intf self, unsigned int ino, uint offset, block_t *block)
 *   - writes *block to the block at the given inode number and offset
 *
 * All these return -1 upon error (typically after printing the eason for
 * the error) and return 0 upon success.
 *
 * An inode_intf also maintains a void* pointer called 'state' to internal
 * state the inode store module needs to keep.
 */

#pragma once
#include "disk.h"

#define NINODES 128
typedef struct inode_store* inode_intf;

struct inode_store {
    int (*getsize)(inode_intf self, uint ino);
    int (*setsize)(inode_intf self, uint ino, uint newsize);
    int (*read)(inode_intf self, uint ino, uint offset, block_t* block);
    int (*write)(inode_intf self, uint ino, uint offset, block_t* block);
    void* state;
};

inode_intf fs_disk_init();

/* There are 2 file systems in egos-2000 right now: mydisk and treedisk. */
inode_intf mydisk_init(inode_intf below, uint below_ino);
int mydisk_create(inode_intf below, uint below_ino, uint ninodes);

inode_intf treedisk_init(inode_intf below, uint below_ino);
int treedisk_create(inode_intf below, uint below_ino, uint ninodes);
