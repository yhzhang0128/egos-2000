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
#include <stdlib.h>
#define DUMMY_DISK_OFFSET(ino, offset) ino * 128 + offset

/* Student's code goes here (File System). */

/* Add a header file similar to file1.h, which defines data structures in your
 * file system. Include this header file and other header files you need, such
 * as string.h. Define helper functions shared by the file system functions. */

/* Student's code ends here. */

int mydisk_read(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (File System). */

    /* Replace the code below with your own file system read logic. */
    inode_intf below = self->state;
    return below->read(below, 0, DUMMY_DISK_OFFSET(ino, offset), block);

    /* Student's code ends here. */
}

int mydisk_write(inode_intf self, uint ino, uint offset, block_t* block) {
    /* Student's code goes here (File System). */

    /* Replace the code below with your own file system write logic. */
    inode_intf below = self->state;
    return below->write(below, 0, DUMMY_DISK_OFFSET(ino, offset), block);

    /* Student's code ends here. */
}

int mydisk_getsize(inode_intf self, uint ino) {
    /* Student's code goes here (File System). */

    /* Replace the error printing with code for getting an inode's size. */
#ifdef MKFS
    fprintf(stderr, "mydisk_getsize not implemented");
    while (1);
#else
    FATAL("mydisk_getsize not implemented");
#endif
    /* Student's code ends here. */
}

int mydisk_setsize(inode_intf self, uint ino, uint nblocks) {
    /* Student's code goes here (File System). */

    /* Replace the error printing with code for changing an inode's size. */
#ifdef MKFS
    fprintf(stderr, "mydisk_setsize not implemented");
    while (1);
#else
    FATAL("mydisk_setsize not implemented");
#endif
    /* Student's code ends here. */
}

int mydisk_create(inode_intf below, uint below_ino, uint ninodes) {
    /* Student's code goes here (File System). */

    /* Create the initial on-disk data structures for your file system. */

    /* Student's code ends here. */
    return 0;
}

inode_intf mydisk_init(inode_intf below, uint below_ino) {
    /* Student's code goes here (File System). */

    /* Feel free to add or modify anything here. */
    inode_intf self = malloc(sizeof(struct inode_store));
    self->getsize   = mydisk_getsize;
    self->setsize   = mydisk_setsize;
    self->read      = mydisk_read;
    self->write     = mydisk_write;
    self->state     = below;
    return self;
    /* Student's code ends here. */
}
