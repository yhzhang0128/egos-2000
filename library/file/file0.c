/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a dummy file system illustrating the concept of "inode"
 */

#ifdef MKFS
#include <stdio.h>
#include <sys/types.h>
#else
#include "egos.h"
#endif
#include "inode.h"
#include <stdlib.h>

#define DUMMY_DISK_OFFSET(ino, offset) ino * 128 + offset

int mydisk_read(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (file system).
     * Read block #offset of inode #ino into block. */
    inode_intf below = self->state;
    return (below->read)(below, 0, DUMMY_DISK_OFFSET(ino, offset), block);
    /* Student's code ends here. */
}

int mydisk_write(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (file system).
     * Write block into block #offset of inode #ino. */
    inode_intf below = self->state;
    return (below->write)(below, 0, DUMMY_DISK_OFFSET(ino, offset), block);
    /* Student's code ends here. */
}

int mydisk_getsize(inode_intf self, uint ino) {
    /* Student's code goes here (file system).
     * Get the size of inode #ino. */
#ifdef MKFS
    fprintf(stderr, "mydisk_getsize not implemented");
    while (1);
#else
    FATAL("mydisk_getsize not implemented");
#endif
    /* Student's code ends here. */
}

int mydisk_setsize(inode_intf self, uint ino, uint nblocks) {
    /* Student's code goes here (file system).
     * Set the size of inode #ino to nblocks. */
#ifdef MKFS
    fprintf(stderr, "mydisk_setsize not implemented");
    while (1);
#else
    FATAL("mydisk_setsize not implemented");
#endif
    /* Student's code ends here. */
}

inode_intf mydisk_init(inode_intf below, uint below_ino) {
    inode_intf self = malloc(sizeof(struct inode_store));
    self->getsize   = mydisk_getsize;
    self->setsize   = mydisk_setsize;
    self->read      = mydisk_read;
    self->write     = mydisk_write;
    self->state     = below;
    return self;
}

int mydisk_create(inode_intf below, uint below_ino, uint ninodes) {
    /* Student's code goes here (file system).
     * Initialize the on-disk data structures for your file system. */

    /* Student's code ends here. */
    return 0;
}
