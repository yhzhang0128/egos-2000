/*
 * (C) 2017, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse, August 2015, updated November 2015
 *
 * This code implements a set of virtualized inode stores on top of a single 
 * inode of another inode store.  Each virtualized inode store is identified by
 * a so-called "inode number", which indexes into an array of inodes.  The 
 * interface is as follows:
 *
 *		void treedisk_create(inode_store_t *below, unsigned int below_ino, unsigned int ninodes)
 *			Initializes the underlying inode store "below" with a file system
 *			stored within inode below_ino. If "below" is a simple
 * 			non-virtualized inode store like the disk server, below_ino is
 * 			probably 0. The file system consists of one "superblock", a number
 * 			of blocks containing inodes, and the remaining blocks explained
 * 			below. The file system can support up to ninodes inodes.
 *
 *		inode_store_t *treedisk_init(inode_store_t *below, unsigned int below_ino)
 *			Opens a virtual inode store within inode below_ino of the inode store below.
 *
 * The layout of the file system is described in the file "file.h".
 */

#include <stdlib.h>
#include <string.h>
#include "file.h"

#ifdef MKFS
#include <stdio.h>
#else
#include "egos.h"
#endif

/* Temporary information about the file system and a particular inode.
 * Convenient for all operations. See "file.h" for field details.
 */
struct treedisk_snapshot {
    union treedisk_block superblock; 
    union treedisk_block inodeblock; 
    block_no inode_blockno;
    struct treedisk_inode *inode;
};

/* The state of a virtual inode store, which is identified by an inode number.
 */
struct treedisk_state {
    inode_store_t *below;			/* inode store below */
    unsigned int below_ino;			/* inode number to use for the inode store below */
    unsigned int ninodes;			/* number of inodes in the treedisk */
};

static unsigned int log_rpb;                    /* log2(REFS_PER_BLOCK) */
static block_t null_block;			/* a block filled with null bytes */

static void panic(const char *s){
#ifdef MKFS
    fprintf(stderr, "%s", s);
    exit(1);
#else 
    FATAL(s);
#endif
}

/* Stupid ANSI C compiler leaves shifting by #bits in unsigned int or more
 * undefined, but the result should clearly be 0...
 */
static block_no log_shift_r(block_no x, unsigned int nbits){
    if (nbits >= sizeof(block_no) * 8) {
        return 0;
    }
    return x >> nbits;
}

/* Get a snapshot of the file system, including the superblock and the block
 * containing the inode, from the inode store below.
 */
static int treedisk_get_snapshot(struct treedisk_snapshot *snapshot,
                                 struct treedisk_state *ts, unsigned int inode_no){
    /* Get the superblock.
     */
    if ((*ts->below->read)(ts->below, ts->below_ino, 0, (block_t *) &snapshot->superblock) < 0)
        return -1;

    /* Check the inode number.
     */
    if (inode_no >= snapshot->superblock.superblock.n_inodeblocks * INODES_PER_BLOCK) {
        printf("!!TDERR: inode number too large %u %u\n", inode_no, snapshot->superblock.superblock.n_inodeblocks);
        return -1;
    }

    /* Find the inode.
     */
    snapshot->inode_blockno = 1 + inode_no / INODES_PER_BLOCK;
    if ((*ts->below->read)(ts->below, ts->below_ino, snapshot->inode_blockno, (block_t *) &snapshot->inodeblock) < 0)
        return -1;

    snapshot->inode = &snapshot->inodeblock.inodeblock.inodes[inode_no % INODES_PER_BLOCK];
    return 0;
}

/* Allocate a block from the free list.
 */
static block_no treedisk_alloc_block(struct treedisk_state *ts, struct treedisk_snapshot *snapshot){
    block_no b;
    static int count;
    count++;

    if ((b = snapshot->superblock.superblock.free_list) == 0)
        panic("treedisk_alloc_block: inode store is full\n");

    /* Read the freelist block and scan for a free block reference.
     */
    union treedisk_block freelistblock;
    (*ts->below->read)(ts->below, ts->below_ino, b, (block_t *) &freelistblock);

    unsigned int i;
    for (i = REFS_PER_BLOCK; --i > 0;)
        if (freelistblock.freelistblock.refs[i] != 0) {
            break;
        }

    /* If there is a free block reference use that.  Otherwise use
     * the free list block itself and update the superblock.
     */
    block_no free_blockno;
    if (i == 0) {
        free_blockno = b;
        snapshot->superblock.superblock.free_list = freelistblock.freelistblock.refs[0];
        if ((*ts->below->write)(ts->below, ts->below_ino, 0, (block_t *) &snapshot->superblock) < 0) {
            panic("treedisk_alloc_block: superblock");
        }
    }
    else {
        free_blockno = freelistblock.freelistblock.refs[i];
        freelistblock.freelistblock.refs[i] = 0;
        if ((*ts->below->write)(ts->below, ts->below_ino, b, (block_t *) &freelistblock) < 0) {
            panic("treedisk_alloc_block: freelistblock");
        }
    }

    return free_blockno;
}

/* Retrieve the number of blocks in the file referenced by 'this_bs'.  This
 * information is maintained in the inode itself.
 */
static int treedisk_getsize(inode_store_t *this_bs, unsigned int ino){
    struct treedisk_state *ts = this_bs->state;
    struct treedisk_snapshot snapshot;
    if (treedisk_get_snapshot(&snapshot, ts, ino) < 0)
        return -1;

    return snapshot.inode->nblocks; 
}

/* Set the size of the file 'this_bs' to 'nblocks'.
 */
static int treedisk_setsize(inode_store_t *this_bs, unsigned int ino, block_no nblocks){
    return -1;
}

/* Read a block at the given block number 'offset' and return in *block.
 */
static int treedisk_read(inode_store_t *this_bs, unsigned int ino, block_no offset, block_t *block){
    struct treedisk_state *ts = this_bs->state;

    /* Get info from underlying file system.
     */
    struct treedisk_snapshot snapshot;
    if (treedisk_get_snapshot(&snapshot, ts, ino) < 0)
        return -1;

    /* See if the offset is too big.
     */
    if (offset >= snapshot.inode->nblocks) {
        /* printf("!!TDERR: offset too large %u %u\n", offset, snapshot.inode->nblocks); */
        return -1;
    }

    /* Figure out how many levels there are in the tree.
     */
    unsigned int nlevels = 0;
    if (snapshot.inode->nblocks > 0)
        while (log_shift_r(snapshot.inode->nblocks - 1, nlevels * log_rpb) != 0) {
            nlevels++;
        }

    /* Walk down from the root block.
     */
    block_no b = snapshot.inode->root;
    for (;;) {
        /* If there's a hole, return the null block.
         */
        if (b == 0) {
            memset(block, 0, BLOCK_SIZE);
            return 0;
        }

        /* Return the next level.  If the last level, we're done.
         */
        int result = (*ts->below->read)(ts->below, ts->below_ino, b, block);
        if (result < 0)
            return result;
        if (nlevels == 0)
            return 0;

        /* The block is an indirect block.  Figure out the index into this
         * block and get the block number.
         */
        nlevels--;
        struct treedisk_indirblock *tib = (struct treedisk_indirblock *) block;
        unsigned int index = log_shift_r(offset, nlevels * log_rpb) % REFS_PER_BLOCK;
        b = tib->refs[index];
    }
    return 0;
}

/* Write *block at the given block number 'offset'.
 */
static int treedisk_write(inode_store_t *this_bs, unsigned int ino, block_no offset, block_t *block){
    struct treedisk_state *ts = this_bs->state;
    int dirty_inode = 0;

    /* Get info from underlying file system.
     */
    struct treedisk_snapshot snapshot_buffer;
    struct treedisk_snapshot *snapshot = &snapshot_buffer;
    if (treedisk_get_snapshot(snapshot, ts, ino) < 0)
        return -1;

    /* Figure out how many levels there are in the tree now.
     */
    unsigned int nlevels = 0;
    if (snapshot->inode->nblocks > 0)
        while (log_shift_r(snapshot->inode->nblocks - 1, nlevels * log_rpb) != 0) {
            nlevels++;
        }

    /* Figure out how many levels we need after writing.  Files cannot shrink
     * by writing.
     */
    unsigned int nlevels_after;
    if (offset >= snapshot->inode->nblocks) {
        snapshot->inode->nblocks = offset + 1;
        dirty_inode = 1;
        nlevels_after = 0;
        while (log_shift_r(offset, nlevels_after * log_rpb) != 0) {
            nlevels_after++;
        }
    }
    else {
        nlevels_after = nlevels;
    }

    /* Grow the number of levels as needed by inserting indirect blocks.
     */
    if (snapshot->inode->nblocks == 0) {
        nlevels = nlevels_after;
    } else if (nlevels_after > nlevels) {
        while (nlevels_after > nlevels) {
            block_no indir = treedisk_alloc_block(ts, snapshot);

            /* Insert the new indirect block into the inode.
             */
            struct treedisk_indirblock tib;
            memset(&tib, 0, BLOCK_SIZE);
            tib.refs[0] = snapshot->inode->root;
            snapshot->inode->root = indir;
            dirty_inode = 1;
            if ((*ts->below->write)(ts->below, ts->below_ino, indir, (block_t *) &tib) < 0) {
                panic("treedisk_write: indirect block");
            }

            nlevels++;
        }
    }

    /* If the inode block was updated, write it back now.
     */
    if (dirty_inode)
        if ((*ts->below->write)(ts->below, ts->below_ino, snapshot->inode_blockno, (block_t *) &snapshot->inodeblock) < 0) {
            panic("treedisk_write: inode block");
        }

    /* Find the block by walking the tree, allocating new blocks
     * (and indirect blocks) if necessary.
     */
    block_no b;
    block_no *parent_no = &snapshot->inode->root;
    block_no parent_off = snapshot->inode_blockno;
    block_t *parent_block = (block_t *) &snapshot->inodeblock;
    for (;;) {
        /* Get or allocate the next block.
         */
        struct treedisk_indirblock tib;
        if ((b = *parent_no) == 0) {
            b = *parent_no = treedisk_alloc_block(ts, snapshot);
            if ((*ts->below->write)(ts->below, ts->below_ino, parent_off, parent_block) < 0)
                panic("treedisk_write: parent");
            if (nlevels == 0)
                break;
            memset(&tib, 0, BLOCK_SIZE);
        }
        else {
            if (nlevels == 0)
                break;
            if ((*ts->below->read)(ts->below, ts->below_ino, b, (block_t *) &tib) < 0)
                panic("treedisk_write");
        }

        /* Figure out the index into this block and get the block number.
         */
        nlevels--;
        unsigned int index = log_shift_r(offset, nlevels * log_rpb) % REFS_PER_BLOCK;
        parent_no = &tib.refs[index];
        parent_block = (block_t *) &tib;
        parent_off = b;
    }

    if ((*ts->below->write)(ts->below, ts->below_ino, b, block) < 0)
        panic("treedisk_write: data block");
    return 0;
}

/* Open a virtual inode store on the specified inode of the inode store below.
 */

inode_store_t *treedisk_init(inode_store_t *below, unsigned int below_ino){
    /* Figure out the log of the number of references per block.
     */
    if (log_rpb == 0) {		/* first time only */
        do {
            log_rpb++;
        } while (((REFS_PER_BLOCK - 1) >> log_rpb) != 0);
    }

    /* Create the inode store state structure.
     */
    struct treedisk_state *ts = malloc(sizeof(struct treedisk_state));
    memset(ts, 0, sizeof(struct treedisk_state));
    ts->below = below;
    ts->below_ino = below_ino;

    /* Return a block interface to this inode.
     */
    inode_store_t *this_bs = malloc(sizeof(inode_store_t));
    memset(this_bs, 0, sizeof(inode_store_t));
    this_bs->state = ts;
    this_bs->getsize = treedisk_getsize;
    this_bs->setsize = treedisk_setsize;
    this_bs->read = treedisk_read;
    this_bs->write = treedisk_write;
    return this_bs;
}

/*************************************************************************
 * The code below is for creating new tree file systems.  This should
 * only be invoked once per underlying inode store.
 ************************************************************************/

/* Create the free list and return the block number of the first
 * block on it.
 */
block_no setup_freelist(inode_store_t *below, unsigned int below_ino, block_no next_free, block_no nblocks){
    block_no freelist_data[REFS_PER_BLOCK];
    block_no freelist_block = 0;
    unsigned int i;

    while (next_free < nblocks) {
        freelist_data[0] = freelist_block;
        freelist_block = next_free++;
        for (i = 1; i < REFS_PER_BLOCK && next_free < nblocks; i++)
            freelist_data[i] = next_free++;

        for (; i < REFS_PER_BLOCK; i++)
            freelist_data[i] = 0;

        if ((*below->write)(below, below_ino, freelist_block, (block_t *) freelist_data) < 0)
            panic("treedisk_setup_freelist");
    }
    return freelist_block;
}

/* Create a new file system on the specified inode of the inode store below.
 */
int treedisk_create(inode_store_t *below, unsigned int below_ino, unsigned int ninodes){
    if (sizeof(union treedisk_block) != BLOCK_SIZE)
        panic("treedisk_create: block has wrong size");

    /* Compute the number of inode blocks needed to store the inodes.
     */
    unsigned int n_inodeblocks = (ninodes + INODES_PER_BLOCK - 1) / INODES_PER_BLOCK;

    /* Get the size of the underlying disk and see if it's large enough.
     */
    unsigned int nblocks = (*below->getsize)(below, below_ino);
    if (nblocks < n_inodeblocks + 2) {
        printf("treedisk_create: too few blocks\n");
        return -1;
    }

    /* Read the superblock to see if it's already initialized.
     */
    union treedisk_block superblock;
    if ((*below->read)(below, below_ino, 0, (block_t *) &superblock) < 0)
        return -1;

    if (superblock.superblock.n_inodeblocks == 0) {
        /* Initialize the superblock.
         */
        union treedisk_block superblock;
        memset(&superblock, 0, BLOCK_SIZE);
        superblock.superblock.n_inodeblocks = n_inodeblocks;
        superblock.superblock.free_list =
            setup_freelist(below, below_ino, n_inodeblocks + 1, nblocks);
        if ((*below->write)(below, below_ino, 0, (block_t *) &superblock) < 0)
            return -1;

        /* The inodes all start out empty.
         */
        for (int i = 1; i <= n_inodeblocks; i++)
            if ((*below->write)(below, below_ino, i, &null_block) < 0) {
                return -1;
            }

        /* printf("treedisk: Created a new filesystem with %d inodes\n", ninodes); */
    }
    else {
        /* printf("treedisk: a filesystem already exists with %lu inodes", */
        /*      superblock.superblock.n_inodeblocks * INODES_PER_BLOCK); */
    }

    return 0;
}
