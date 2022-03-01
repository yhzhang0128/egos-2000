/*
 * (C) 2017, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse, August 2015
 *
 * This file describes the layout of a treedisk file system.  A file is
 * a virtualized block store.  Each virtualized file is identified by a
 * so-called "inode number", which indexes into an array of inodes.
 *
 * The superblock maintains the number of inode blocks and a pointer
 * to the free list structure.
 *
 * An inode block is filled with INODES_PER_BLOCK inodes.  Data in the
 * inode is stored in a complete tree, with the branching vector determined
 * by the number of block indices that fit in a block (REFS_PER_BLOCK).
 * All data blocks are at the bottom level.  Each inode contains the number
 * of blocks in the virtual block store, and block index of the "root block".
 * If the number of blocks in the virtual store is exactly one, this root
 * block simply contains the data.  If larger than one, then the root
 * block is an "indirect block", filled with block indexes to other blocks.
 *
 * An exception is that the block index 0 is used for "holes".  These can
 * exist both for data and indirect blocks.  Reading from a hole returns
 * null bytes.
 *
 * The free list is a linked list of blocks.  Each block is filled with
 * block indices, the first of which is either 0 to indicate the end of
 * the list, or otherwise a pointer to the next block on the list.  The
 * remaining slots point to free blocks, or 0 if the slot is empty.
 */

#define INODES_PER_BLOCK	(BLOCK_SIZE / sizeof(struct treedisk_inode))
#define REFS_PER_BLOCK		(BLOCK_SIZE / sizeof(block_no))

/* Contents of the "superblock".  There is only one of these.
 */
struct treedisk_superblock {
	block_no n_inodeblocks;		// # blocks with inodes
	block_no free_list;			// pointer to first block on free list
};

/* An inode describes a file (= virtual block store).  "nblocks" contains
 * the number of blocks in the file, while "root" is the top most block in
 * the tree of blocks.  Note that initially "all files exist" but are of
 * length 0.  It is intended that keeping track which files are free or
 * not is maintained elsewhere.
 */
struct treedisk_inode {
	block_no root;				// block number of root node
	block_no nblocks;			// total size of the file
};

/* An inode block is filled with inodes.
 */
struct treedisk_inodeblock {
	struct treedisk_inode inodes[INODES_PER_BLOCK];
};

/* A freelist block is filled with references to other blocks, the first
 * one of which is the next freelist block (0 = end-of-list). Remember that
 * the freelist acts as a stack (freelist blocks are added FILO).
 */
struct treedisk_freelistblock {
	block_no refs[REFS_PER_BLOCK];
};

/* An indirect block is an internal node in the tree rooted at an inode.
 */
struct treedisk_indirblock {
	block_no refs[REFS_PER_BLOCK];
};

/* A convenient structure that's the union of all block types.  It should
 * have size BLOCK_SIZE, which may not be true for the elements.
 */
union treedisk_block {
	block_t datablock;
	struct treedisk_superblock superblock;
	struct treedisk_inodeblock inodeblock;
	struct treedisk_freelistblock freelistblock;
	struct treedisk_indirblock indirblock;
};
