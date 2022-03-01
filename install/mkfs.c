/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the disk image file (disk.img)
 * the first 1MB is reserved as 256 physical frames for paging
 * the second 1MB contains some ELF binary executables for booting
 * the last 4MB is managed by a file system
 * in total, disk.img should be 6MB
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "disk.h"
#include "fs.h"
#include "treedisk.h"

#define NKERNEL_PROC 4
char* kernel_processes[] = {
                            "bin/release/grass.elf",
                            "bin/release/sys_proc.elf",
                            "bin/release/sys_file.elf",
                            "bin/release/sys_dir.elf",
                            //"bin/release/sys_shell.elf"
};

#define NINODE 5

/* inode mappings:
#0: /
#1: /home
#2: /home/yunhao
#3: /home/rvr
#4: /home/yunhao/README
 */

char* contents[] = {
                    ".   0 ..   0 home   1 \n",
                    ".   1 ..   0 yunhao   2 rvr   3 \n",
                    ".   2 ..   1 README   4 ",
                    ".   3 ..   1 ",
                    "This is the README file of egos-riscv!"
};

char fs[FS_DISK_SIZE];
char exec[GRASS_EXEC_SIZE];
char paging[PAGING_DEV_SIZE];

void mkfs();

int main() {
    freopen("disk.img", "w", stdout);

    /* paging area */
    memset(paging, 0, PAGING_DEV_SIZE);
    write(1, exec, PAGING_DEV_SIZE);

    /* grass kernel processes */
    int n = NKERNEL_PROC;
    if (n > GRASS_NEXEC) {
        fprintf(stderr, "[ERROR] >%d kernel processes\n", GRASS_NEXEC);
        return -1;
    }

    int exec_size = GRASS_EXEC_SIZE / GRASS_NEXEC;
    fprintf(stderr, "[INFO] Loading %d kernel processes\n", n);
    for (int i = 0; i < n; i++) {
        memset(exec, 0, GRASS_EXEC_SIZE);
        
        struct stat st;
        stat(kernel_processes[i], &st);
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", kernel_processes[i], (long)st.st_size);
        assert(st.st_size > 0);

        if (st.st_size > exec_size) {
            fprintf(stderr, "[ERROR] file larger than 128KB\n");
            return -1;
        }

        freopen(kernel_processes[i], "r", stdin);
        int nread = 0;
        while (nread < st.st_size)
            nread += read(0, exec + nread, exec_size - nread);

        write(1, exec, st.st_size);
        write(1, exec, exec_size - st.st_size);
    }

    memset(exec, 0, GRASS_EXEC_SIZE);
    for (int i = 0; i < 8 - n; i++)
        write(1, exec, exec_size);
        
    /* file system */
    memset(fs, 0, FS_DISK_SIZE);
    mkfs();
    write(1, fs, FS_DISK_SIZE);
    
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image (size=%d)\n", FS_DISK_SIZE + GRASS_EXEC_SIZE + PAGING_DEV_SIZE);
    return 0;
}


block_if ramdisk_init();

void mkfs() {
    fprintf(stderr, "[INFO] Making the file system with treedisk\n");

    block_if ramdisk = ramdisk_init();    
    if (treedisk_create(ramdisk, 0, NINODES) < 0) {
        fprintf(stderr, "proc_file: can't create treedisk file system");
        exit(1);
    }
    block_if treedisk = treedisk_init(ramdisk, 0);

    char buf[BLOCK_SIZE];
    for (int ino = 0; ino < NINODE; ino++) {
        strncpy(buf, contents[ino], BLOCK_SIZE);
        treedisk->write(treedisk, ino, 0, (void*)buf);
    }
    fprintf(stderr, "[INFO] Write %d inodes\n", NINODE);
}


int getsize(block_if this_bs, unsigned int ino){
    return FS_DISK_SIZE / BLOCK_SIZE;
}

int setsize(block_if this_bs, unsigned int ino, block_no newsize) {
    fprintf(stderr, "disk_setsize not implemented");
    exit(1);
}

int ramread(block_if this_bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(block_if this_bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(fs + offset * BLOCK_SIZE, block, BLOCK_SIZE);
    return 0;
}

block_if ramdisk_init() {
    block_store_t *ramdisk = malloc(sizeof(*ramdisk));

    ramdisk->read = ramread;
    ramdisk->write = ramwrite;
    ramdisk->getsize = getsize;
    ramdisk->setsize = setsize;

    return ramdisk;
}

