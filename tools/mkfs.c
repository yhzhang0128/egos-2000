/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the disk image file (disk.img)
 * The disk image should be exactly 4MB:
 *     the first 1MB is reserved as 256 frames for memory paging;
 *     the next  1MB contains some ELF binary executables for booting;
 *     the last  2MB is managed by a file system.
 * The output is in binary format (disk.img).
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
                            "../build/release/sys_shell.elf",
};

/* Inode mappings:
#0: /           #1: /home          #2: /home/yunhao
#3: /home/rvr   #4: /home/lorenzo  #5: /home/yunhao/README 
#6: /bin        #7: /bin/echo      #8: /bin/cat
#9: /bin/ls     #10: /bin/cd       #11:/bin/pwd
#12:/bin/clock  #13: /bin/crash1   #14: /bin/crash2
*/
#define NINODE 15
char* contents[] = {
                    "./   0 ../   0 home/   1 bin/   6 ",
                    "./   1 ../   0 yunhao/   2 rvr/   3 lorenzo/   4 ",
                    "./   2 ../   1 README   5 ",
                    "./   3 ../   1 ",
                    "./   4 ../   1 ",
                    "With only 2000 lines of code, egos-2000 implements boot loader, microSD driver, tty driver, memory paging, address translation, interrupt handling, process scheduling and messaging, system call, file system, shell, 7 user commands and the `mkfs/mkrom` tools.",
                    "./   6 ../   0 echo   7 cat   8 ls   9 cd  10 pwd  11 clock  12 crash1  13 crash2  14 ",
                    "#../build/release/echo.elf",
                    "#../build/release/cat.elf",
                    "#../build/release/ls.elf",
                    "#../build/release/cd.elf",
                    "#../build/release/pwd.elf",
                    "#../build/release/clock.elf",
                    "#../build/release/crash1.elf",
                    "#../build/release/crash2.elf",
};

char fs[FS_DISK_SIZE];
char exec[GRASS_EXEC_SIZE];
char paging[PAGING_DEV_SIZE];

void mkfs();
inode_intf ramdisk_init();

int main() {
    mkfs();

    /* Paging area */
    freopen("disk.img", "w", stdout);
    write(1, exec, PAGING_DEV_SIZE);

    /* Grass kernel processes */
    int exec_size = GRASS_EXEC_SIZE / GRASS_NEXEC;
    fprintf(stderr, "[INFO] Loading %d kernel binary files\n", NKERNEL_PROC);

    for (int i = 0; i < NKERNEL_PROC; i++) {
        struct stat st;
        stat(kernel_processes[i], &st);
        assert((st.st_size > 0) && (st.st_size <= exec_size));
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", kernel_processes[i], (long)st.st_size);

        freopen(kernel_processes[i], "r", stdin);
        memset(exec, 0, GRASS_EXEC_SIZE);
        for (int nread = 0; nread < st.st_size; )
            nread += read(0, exec + nread, exec_size - nread);

        write(1, exec, st.st_size);
        write(1, exec, exec_size - st.st_size);
    }
    write(1, exec, (GRASS_NEXEC - NKERNEL_PROC) * exec_size);
        
    /* File system */
    write(1, fs, FS_DISK_SIZE);
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image\n");
    return 0;
}


void mkfs() {
    inode_intf ramdisk = ramdisk_init();
    assert(treedisk_create(ramdisk, 0, NINODES) >= 0);
    inode_intf treedisk = treedisk_init(ramdisk, 0);

    char buf[GRASS_EXEC_SIZE / GRASS_NEXEC];
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
            for (int nread = 0; nread < st.st_size; )
                nread += read(0, buf + nread, st.st_size - nread);
            
            fprintf(stderr, "[INFO] Loading ino=%d, %s: %d bytes\n", ino, file_name, (int)st.st_size);
            for (int b = 0; b * BLOCK_SIZE < st.st_size; b++)
                treedisk->write(treedisk, ino, b, (void*)(buf + b * BLOCK_SIZE));
        }
    }
    fprintf(stderr, "[INFO] Write %d inodes\n", NINODE);
}


int getsize() { return FS_DISK_SIZE / BLOCK_SIZE; }

int setsize() { assert(0); }

int ramread(inode_intf bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(inode_intf bs, unsigned int ino, block_no offset, block_t *block) {
    memcpy(fs + offset * BLOCK_SIZE, block, BLOCK_SIZE);
    return 0;
}

inode_intf ramdisk_init() {
    inode_store_t *ramdisk = malloc(sizeof(*ramdisk));

    ramdisk->read = (void*)ramread;
    ramdisk->write = (void*)ramwrite;
    ramdisk->getsize = (void*)getsize;
    ramdisk->setsize = (void*)setsize;

    return ramdisk;
}

