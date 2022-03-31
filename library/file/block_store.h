#pragma once
/*
 * (C) 2017, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse, August 2015
 *
 * This is the include file for all block store modules.  A block store
 * maintains one or more "inodes", each representing an array of blocks.
 * A physical disk would typically just have one inode (inode 0), while
 * a virtualized disk may have many.  Each block store module has an
 * 'init' function that returns a block_store_t *.  The block_store_t * is
 * a pointer to a structure that contains the following seven methods:
 *
 *      int getsize(block_store_t *this_bs, unsigned int ino)
 *          returns the size of the block store at the given inode number
 *			(inode numbers start at 0)
 *
 *      int setsize(block_store_t *this_bs, unsigned int ino, block_no newsize)
 *          set the size of the block store at the given inode number
 *          returns the old size
 *
 *      int read(block_store_t *this_bs, unsigned int ino, block_no offset, block_t *block)
 *          read the block at the given inode number and offset and return in *block
 *          returns 0
 *
 *      int write(block_store_t *this_bs, unsigned int ino, block_no offset, block_t *block)
 *          write *block to the block at the given inode number and offset
 *          returns 0
 *
 * All these return -1 upon error (typically after printing the
 * reason for the error).
 *
 * A 'block_t' is a block of BLOCK_SIZE bytes.  A block store is an array
 * of blocks.  A 'block_no' holds the index of the block in the block store.
 *
 * A block_store_t * also maintains a void* pointer called 'state' to internal
 * state the block store module needs to keep.
 */

#include "disk.h"

#define NINODES  128
typedef unsigned int block_no;      // index of a block

typedef struct block {
    char bytes[BLOCK_SIZE];
} block_t;

typedef struct block_store {
    void *state;
    int (*getsize)(struct block_store *this_bs, unsigned int ino);
    int (*setsize)(struct block_store *this_bs, unsigned int ino, block_no newsize);
    int (*read)(struct block_store *this_bs, unsigned int ino, block_no offset, block_t *block);
    int (*write)(struct block_store *this_bs, unsigned int ino, block_no offset, block_t *block);
} block_store_t;

typedef block_store_t *block_if;    // block store interface

block_if fs_disk_init();
block_if treedisk_init(block_if below, unsigned int below_ino);
int treedisk_create(block_if below, unsigned int below_ino, unsigned int ninodes);
