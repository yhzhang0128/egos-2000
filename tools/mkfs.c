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
#include "file.h"

#define NKERNEL_PROC 5
char* kernel_processes[] = {
                            "../build/release/grass.elf",
                            "../build/release/sys_proc.elf",
                            "../build/release/sys_file.elf",
                            "../build/release/sys_dir.elf",
                            "../build/release/sys_shell.elf"
};

#define NINODE 10

/* inode mappings:
#0: /              #1: /home               #2: /home/yunhao
#3: /home/rvr      #4: /home/yunhao/README #5: /bin
#6: /bin/echo      #7: /bin/ls             #8: /bin/cat
#9: /bin/clock
*/

char* contents[] = {
                    ".   0 ..   0 home   1 bin   5 ",
                    ".   1 ..   0 yunhao   2 rvr   3 ",
                    ".   2 ..   1 README   4 ",
                    ".   3 ..   1 ",
                    "With only 2.3K lines of code, egos-riscv implements SD card driver, tty driver, interrupt handling, address translation, process scheduling and communication, system calls, file system, shell and 7 shell commands.",
                    ".   5 ..   0 echo   6 ls   7 cat   8 clock   9 ",
                    "#../build/release/echo.elf",
                    "#../build/release/ls.elf",
                    "#../build/release/cat.elf",
                    "#../build/release/clock.elf",
};
/*NOTICE: in a dir, *4* bytes following the name gives the inode number*/

char fs[FS_DISK_SIZE];
char exec[GRASS_EXEC_SIZE];
char paging[PAGING_DEV_SIZE];

void mkfs();

int main() {
    memset(fs, 0, FS_DISK_SIZE);
    mkfs();

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
    fprintf(stderr, "[INFO] Loading %d kernel binary files\n", n);
    for (int i = 0; i < n; i++) {
        memset(exec, 0, GRASS_EXEC_SIZE);
        
        struct stat st;
        stat(kernel_processes[i], &st);
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", kernel_processes[i], (long)st.st_size);
        assert(st.st_size > 0);

        if (st.st_size > exec_size) {
            fprintf(stderr, "[ERROR] file larger than %d\n", exec_size);
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
    write(1, fs, FS_DISK_SIZE);
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image\n");
    return 0;
}


block_if ramdisk_init();

void mkfs() {
    block_if ramdisk = ramdisk_init();    
    if (treedisk_create(ramdisk, 0, NINODES) < 0) {
        fprintf(stderr, "proc_file: can't create treedisk file system");
        exit(1);
    }
    block_if treedisk = treedisk_init(ramdisk, 0);

    char buf[BLOCK_SIZE];
    for (int ino = 0; ino < NINODE; ino++) {
        if (contents[ino][0] != '#') {
            fprintf(stderr, "[INFO] Loading ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
            strncpy(buf, contents[ino], BLOCK_SIZE);
            treedisk->write(treedisk, ino, 0, (void*)buf);
        } else {
            struct stat st;
            char* file_name = &contents[ino][1];
            stat(file_name, &st);
            
            freopen(file_name, "r", stdin);
            int nread = 0;
            char file[GRASS_EXEC_SIZE / GRASS_NEXEC];
            while (nread < st.st_size)
                nread += read(0, file + nread, st.st_size - nread);
            
            fprintf(stderr, "[INFO] Loading ino=%d, %s: %d bytes\n", ino, file_name, nread);
            for (int b = 0; b * BLOCK_SIZE < st.st_size; b++) {
                treedisk->write(treedisk, ino, b, (void*)(file + b * BLOCK_SIZE));
            }
        }
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

