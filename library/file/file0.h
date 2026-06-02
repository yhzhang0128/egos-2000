#pragma once
#include "inode.h"

typedef unsigned int block_no; /* index of a block */
#define BLOCK_NONE ((block_no)-1)
#define REFS_PER_BLOCK   (BLOCK_SIZE / sizeof(block_no))
#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(struct linkedlist_inode))
#define FAT_ENTRIES_PER_BLOCK (BLOCK_SIZE / sizeof(struct FAT_entry))
/* Contents of the "superblock".  There is only one of these.
 */

typedef struct linkedlist_freelistnode{
    block_no next; // block number of next node in the free list, or 0 if this is the last node
} linkedlist_freelistnode;

typedef struct linkedlist_superblock {
    block_no n_inodeblocks; /* # blocks with inodes */
    block_no free_list;     /* block number of first block on free list */
    block_no n_FATblocks;   /* # FAT blocks */
} linkedlist_superblock;

typedef struct linkedlist_inode {
    block_no head; /* block number of head of linked list */
    block_no nblocks; // number of blocks in the inode linkedlist
} linkedlist_inode;

typedef struct FAT_entry {
    block_no next; // block number of next block in the linked list, or BLOCK_NONE if this is the last block
} FAT_entry; 

typedef struct FAT_block {
    FAT_entry entries[FAT_ENTRIES_PER_BLOCK];
} FAT_block;