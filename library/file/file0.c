/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: a simple (if not naive) file system
 */

#ifdef MKFS
#include <stdio.h>
#include <sys/types.h>
#else
#include "egos.h"
#endif

#include "inode.h"
#include "file0.h"
#include <stdlib.h>
#include <string.h>
// initial implementation assumes that each inode stores 128 blocks
#define DUMMY_DISK_OFFSET(ino, offset) ino * 128 + offset

/* Student's code goes here (File System). */

/* Include necessary header files; Define data structures for your file system;
   Define helper functions; You can also put these to a separate header file. */

/* Student's code ends here. */

int mydisk_read(inode_intf self, uint ino, uint offset, block_t* dst_block) {
    inode_intf below = self->state;
    block_t superblock_block;
    memset(&superblock_block, 0, sizeof(superblock_block));
    if (below->read(below, 0, 0, &superblock_block) < 0) return -1;
    linkedlist_superblock* superblock = (linkedlist_superblock*)superblock_block.bytes;

    if (ino >= superblock->n_inodeblocks * INODES_PER_BLOCK) {
        printf("mydisk_read: inode number out of range\n");
        return -1;
    }

    block_t inode_block;
    memset(&inode_block, 0, sizeof(inode_block));
    if (below->read(below, 0, ino / INODES_PER_BLOCK + 1, &inode_block) < 0) return -1;
    linkedlist_inode* inodes = (linkedlist_inode*)inode_block.bytes;
    linkedlist_inode* inode = &inodes[ino % INODES_PER_BLOCK];

    if (offset >= inode->nblocks) {
        printf("mydisk_read: offset out of range\n");
        return -1;
    }

    block_no current_block_no = inode->head;
    for (uint i = 0; i < offset; i++) {
        block_t FAT_block;
        memset(&FAT_block, 0, sizeof(FAT_block));
        if (below->read(below, 0,
                        1 + superblock->n_inodeblocks + current_block_no / FAT_ENTRIES_PER_BLOCK,
                        &FAT_block) < 0) return -1;

        FAT_entry* entries = (FAT_entry*)FAT_block.bytes;
        current_block_no = entries[current_block_no % FAT_ENTRIES_PER_BLOCK].next;
    }

    return below->read(below, 0, current_block_no, dst_block);
}
//underlying disk is just one big inode, so it should be below->read(below, 0, offset)
int mydisk_write(inode_intf self, uint ino, uint offset, block_t* dst_block) {
    inode_intf below = self->state;
    block_t superblock_block;
    memset(&superblock_block, 0, sizeof(superblock_block));
    if (below->read(below, 0, 0, &superblock_block) < 0) return -1;
    linkedlist_superblock* superblock = (linkedlist_superblock*)superblock_block.bytes;

    if (ino >= superblock->n_inodeblocks * INODES_PER_BLOCK) {
        printf("mydisk_write: inode number out of range\n");
        return -1;
    }

    block_t inode_block;
    memset(&inode_block, 0, sizeof(inode_block));
    if (below->read(below, 0, ino / INODES_PER_BLOCK + 1, &inode_block) < 0) return -1;
    linkedlist_inode* inodes = (linkedlist_inode*)inode_block.bytes;
    linkedlist_inode* inode = &inodes[ino % INODES_PER_BLOCK];

    if (offset < inode->nblocks) {
        block_no current_block_no = inode->head;
        for (uint i = 0; i < offset; i++) {
            block_t FAT_block;
            memset(&FAT_block, 0, sizeof(FAT_block));
            if (below->read(below, 0,
                            1 + superblock->n_inodeblocks + current_block_no / FAT_ENTRIES_PER_BLOCK,
                            &FAT_block) < 0) return -1;

            FAT_entry* entries = (FAT_entry*)FAT_block.bytes;
            current_block_no = entries[current_block_no % FAT_ENTRIES_PER_BLOCK].next;
        }
        return below->write(below, 0, current_block_no, dst_block);
    }

    uint blocks_to_add = offset - inode->nblocks + 1;
    if (inode->head == BLOCK_NONE) {
        block_no free_block_no = superblock->free_list;
        if (free_block_no == BLOCK_NONE) {
            printf("mydisk_write: no more free blocks\n");
            return -1;
        }

        block_t FAT_block;
        memset(&FAT_block, 0, sizeof(FAT_block));
        if (below->read(below, 0,
                        1 + superblock->n_inodeblocks + free_block_no / FAT_ENTRIES_PER_BLOCK,
                        &FAT_block) < 0) return -1;
        FAT_entry* entries = (FAT_entry*)FAT_block.bytes;

        inode->head = free_block_no;
        inode->nblocks = 1;
        superblock->free_list = entries[free_block_no % FAT_ENTRIES_PER_BLOCK].next;
        entries[free_block_no % FAT_ENTRIES_PER_BLOCK].next = BLOCK_NONE;
        blocks_to_add--;

        if (below->write(below, 0,
                         1 + superblock->n_inodeblocks + free_block_no / FAT_ENTRIES_PER_BLOCK,
                         &FAT_block) < 0) return -1;

        if (blocks_to_add > 0) {
            block_t zero_block;
            memset(&zero_block, 0, sizeof(zero_block));
            if (below->write(below, 0, free_block_no, &zero_block) < 0) return -1;
        }
    }

    block_no tail = inode->head;
    while (1) {
        block_t FAT_block;
        memset(&FAT_block, 0, sizeof(FAT_block));
        if (below->read(below, 0,
                        1 + superblock->n_inodeblocks + tail / FAT_ENTRIES_PER_BLOCK,
                        &FAT_block) < 0) return -1;
        FAT_entry* entries = (FAT_entry*)FAT_block.bytes;
        if (entries[tail % FAT_ENTRIES_PER_BLOCK].next == BLOCK_NONE) break;
        tail = entries[tail % FAT_ENTRIES_PER_BLOCK].next;
    }

    for (uint i = 0; i < blocks_to_add; i++) {
        block_no free_block_no = superblock->free_list;
        if (free_block_no == BLOCK_NONE) {
            printf("mydisk_write: no more free blocks\n");
            return -1;
        }

        block_t FAT_block_free_node;
        memset(&FAT_block_free_node, 0, sizeof(FAT_block_free_node));
        if (below->read(below, 0,
                        1 + superblock->n_inodeblocks + free_block_no / FAT_ENTRIES_PER_BLOCK,
                        &FAT_block_free_node) < 0) return -1;
        FAT_entry* free_entries = (FAT_entry*)FAT_block_free_node.bytes;

        FAT_entry* tail_entries;
        block_t FAT_block_tail;
        if (free_block_no / FAT_ENTRIES_PER_BLOCK == tail / FAT_ENTRIES_PER_BLOCK) {
            tail_entries = free_entries;
        } else {
            memset(&FAT_block_tail, 0, sizeof(FAT_block_tail));
            if (below->read(below, 0,
                            1 + superblock->n_inodeblocks + tail / FAT_ENTRIES_PER_BLOCK,
                            &FAT_block_tail) < 0) return -1;
            tail_entries = (FAT_entry*)FAT_block_tail.bytes;
        }

        tail_entries[tail % FAT_ENTRIES_PER_BLOCK].next = free_block_no;
        inode->nblocks++;
        superblock->free_list = free_entries[free_block_no % FAT_ENTRIES_PER_BLOCK].next;
        free_entries[free_block_no % FAT_ENTRIES_PER_BLOCK].next = BLOCK_NONE;

        if (below->write(below, 0,
                         1 + superblock->n_inodeblocks + free_block_no / FAT_ENTRIES_PER_BLOCK,
                         &FAT_block_free_node) < 0) return -1;
        if (tail / FAT_ENTRIES_PER_BLOCK != free_block_no / FAT_ENTRIES_PER_BLOCK) {
            if (below->write(below, 0,
                             1 + superblock->n_inodeblocks + tail / FAT_ENTRIES_PER_BLOCK,
                             &FAT_block_tail) < 0) return -1;
        }

        tail = free_block_no;
        if (i + 1 < blocks_to_add) {
            block_t zero_block;
            memset(&zero_block, 0, sizeof(zero_block));
            if (below->write(below, 0, tail, &zero_block) < 0) return -1;
        }
    }

    if (below->write(below, 0, 0, &superblock_block) < 0) return -1;
    if (below->write(below, 0, 1 + ino / INODES_PER_BLOCK, &inode_block) < 0) return -1;
    return below->write(below, 0, tail, dst_block);
}


int mydisk_getsize(inode_intf self, uint ino) {
    /* Student's code goes here (File System). */

    /* Replace the code below with code for getting the size of an inode. */
#ifdef MKFS
    fprintf(stderr, "mydisk_getsize not implemented");
    while (1);
#else
    printf("mydisk_getsize not implemented");
#endif
    /* Student's code ends here. */
}

int mydisk_setsize(inode_intf self, uint ino, uint nblocks) {
    /* Student's code goes here (File System). */

    /* Replace the code below with code for changing the size of an inode. */
#ifdef MKFS
    fprintf(stderr, "mydisk_setsize not implemented");
    while (1);
#else
    printf("mydisk_setsize not implemented");
#endif
    /* Student's code ends here. */
}

int mydisk_create(inode_intf below, uint below_ino, uint ninodes) {
    int below_nblocks = below->getsize(below, below_ino);
    if (below_nblocks < 0) return -1;

    block_no nblocks = below_nblocks;
    block_no n_inodeblocks =
        (ninodes + INODES_PER_BLOCK - 1) / INODES_PER_BLOCK;
    // need an entry in each FAT block for each data block in underlying disk, DUHH
    block_no n_FATblocks =
        (nblocks + FAT_ENTRIES_PER_BLOCK - 1) / FAT_ENTRIES_PER_BLOCK;
    block_no first_data_block = 1 + n_inodeblocks + n_FATblocks;

    if (nblocks <= first_data_block) {
        printf("mydisk_create: too few blocks\n");
        return -1;
    }

    block_t superblock_block;
    memset(&superblock_block, 0, sizeof(superblock_block));
    linkedlist_superblock* superblock =
        (linkedlist_superblock*)superblock_block.bytes;
    superblock->n_inodeblocks = n_inodeblocks;
    superblock->n_FATblocks = n_FATblocks;
    superblock->free_list = first_data_block; // free list is sequential??

    if (below->write(below, below_ino, 0, &superblock_block) < 0) {    
        printf("mydisk_create: write to underlying disk failed\n");
        return -1;
    }

    for (block_no i = 1; i <=n_inodeblocks; i++) {
        block_t inode_block; // define a type for block of memory, and then we cast to write into it OHHH
        memset(&inode_block, 0, sizeof(inode_block));
        linkedlist_inode* inodes = (linkedlist_inode*)inode_block.bytes;

        for (uint j = 0; j < INODES_PER_BLOCK; j++) {
            inodes[j].head = BLOCK_NONE;
            inodes[j].nblocks = 0;
        }

        if (below->write(below, below_ino, i, &inode_block) < 0){
            printf("mydisk_create: write to underlying disk failed for inode %d\n", i);
            return -1;
        }
    }
    // FAT is meant for linkedlist of blocks that a given inode stores; in this instance; freeList stores the FAT blocks that arent in use?
    // it just stores chains of blocks
    for (block_no i = 0; i < n_FATblocks; i++) {
        block_t FAT_block;
        memset(&FAT_block, 0, sizeof(FAT_block));
        FAT_entry* entries = (FAT_entry*)FAT_block.bytes; //array of FAT entries, within the block

        for (uint j = 0; j < FAT_ENTRIES_PER_BLOCK; j++) { // initially, FAT entries compose the global free list
            block_no FAT_block_no = i * FAT_ENTRIES_PER_BLOCK + j;
            block_no next_FAT_block_no = FAT_block_no + 1;
            //entries before first_data_block are reserved for superblock and inodes, so we skip those
            if (first_data_block <= FAT_block_no && FAT_block_no < nblocks) {
                entries[j].next = next_FAT_block_no < nblocks ? next_FAT_block_no : BLOCK_NONE;
            } 
            else {
                entries[j].next = BLOCK_NONE;
            }

        }
        if (below->write(below, below_ino, 1 + n_inodeblocks + i, &FAT_block) < 0){
            printf("mydisk_create: write to underlying disk failed for FAT block %d\n", i);
            return -1;
        }
    }

    printf("mydisk: Created a new filesystem with %d inodes\n", ninodes);
    return 0;
}

inode_intf mydisk_init(inode_intf below, uint below_ino) {
    (void)below_ino;
    inode_intf self = malloc(sizeof(struct inode_store));
    self->getsize = mydisk_getsize;
    self->setsize = mydisk_setsize;
    self->read = mydisk_read;
    self->write = mydisk_write;
    self->state = below;
    return self;
}

/*
The mydisk_getsize and mydisk_setsize functions simply print out an error and halt, meaning that they are not used when booting egos-2000. You will start from here and implement your own file system in the mydisk_* functions in file0.c. Make sure to set FILESYS = 0 in Makefile, so mkfs and egos-2000 run your file system code. Specifically, after make install, you should see MKFS is using *mydisk* instead of MKFS is using *treedisk*.

We proceed to implement sumn similair to File Allocation Table (FAT) file system, which is a simple and widely used file system

Linked List:
- standard stuff
- NOTE, we do not define it in C like usual, it will be stored in memory. instead, we must write it to disk, so the info
still persists even after powering off computer

Storing on disk:
- split disk into four regions
    1. Super block
    2. inode blocks
    3. FAT blocks
    4. data blocks

*/
