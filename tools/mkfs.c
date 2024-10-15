/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: create the disk image file (disk.img)
 * The disk image should be 4MB:
 *     2MB contains the executables of EGOS and system servers;
 *     2MB is managed by a file system.
 * This image file should be programmed to the microSD card.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "inode.h"

#define EGOS_BIN_NUM 5
char* egos_binaries[] = {"./qemu/egos.bin",
                         "../build/release/sys_process.elf",
                         "../build/release/sys_terminal.elf",
                         "../build/release/sys_file.elf",
                         "../build/release/sys_shell.elf"};

#define BIN_DIR_INODE 6
char bin_dir[128] = "./   6 ../   0 ";
char* contents[] = {"./   0 ../   0 home/   1 bin/   6 ",
                    "./   1 ../   0 yunhao/   2 rvr/   3 yacqub/   4 ",
                    "./   2 ../   1 README   5 ",
                    "./   3 ../   1 ",
                    "./   4 ../   1 ",
                    "With only 2000 lines of code, egos-2000 implements boot loader, microSD driver, tty driver, memory translation, interrupt handling, preemptive scheduler, system call, file system, shell, a UDP/Ethernet demo, several user commands, and the `mkfs/mkrom` tools.",
                    bin_dir};

char exec[EGOS_BIN_MAX_NBLOCK * BLOCK_SIZE], fs[FILE_SYS_DISK_SIZE], buf[EGOS_BIN_MAX_NBLOCK * BLOCK_SIZE], elf_pathname[128];

inode_intf ramdisk_init();

int main() {
    /* Make the file system into char fs[] */
    inode_intf ramdisk = ramdisk_init();
    (FILESYS == 0)? assert(mydisk_create(ramdisk, 0, NINODES) >= 0) :
                    assert(treedisk_create(ramdisk, 0, NINODES) >= 0);
    inode_intf filesys = (FILESYS == 0)? mydisk_init(ramdisk, 0) :
                                         treedisk_init(ramdisk, 0);
    fprintf(stderr, "MKFS is using file system: %s\n", FILESYS == 0? "mydisk" : "treedisk");

    for (uint ino = 0; ino < BIN_DIR_INODE; ino++) {
        fprintf(stderr, "[INFO] Loading ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
        strncpy(buf, contents[ino], BLOCK_SIZE);
        filesys->write(filesys, ino, 0, (void*)buf);
    }

    uint app_ino = BIN_DIR_INODE + 1;
    DIR *dp = opendir ("../build/release/user");
    assert( dp != NULL );
    for (struct dirent *ep = readdir (dp); ep != NULL; ep = readdir (dp))
        if (strstr(ep->d_name, ".elf")) {
            sprintf(elf_pathname, "../build/release/user/%s", ep->d_name);
            struct stat st;
            stat(elf_pathname, &st);

            freopen(elf_pathname, "r", stdin);
            for (uint nread = 0; nread < st.st_size; )
                nread += read(0, buf + nread, st.st_size - nread);

            fprintf(stderr, "[INFO] Loading ino=%d, %s: %d bytes\n", app_ino, elf_pathname, (int)st.st_size);
            for (uint b = 0; b * BLOCK_SIZE < st.st_size; b++)
                filesys->write(filesys, app_ino, b, (void*)(buf + b * BLOCK_SIZE));

            ep->d_name[strlen(ep->d_name) - 4] = 0;
            sprintf(buf, "%s%4d ", ep->d_name, app_ino++);
            strcat(bin_dir, buf);
        }
    closedir (dp);
    filesys->write(filesys, BIN_DIR_INODE, 0, (void*)bin_dir);
    printf("[INFO] Loading ino=%d, %s\n", BIN_DIR_INODE, bin_dir);

    /* Write EGOS binaries into disk.img */
    freopen("disk.img", "w", stdout);
    fprintf(stderr, "[INFO] Loading %d kernel binary files\n", EGOS_BIN_NUM);

    for (uint i = 0; i < EGOS_BIN_NUM; i++) {
        struct stat st;
        stat(egos_binaries[i], &st);
        assert((st.st_size > 0) && (st.st_size <= sizeof(exec)));
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", egos_binaries[i], (long)st.st_size);

        freopen(egos_binaries[i], "r", stdin);
        memset(exec, 0, sizeof(exec));
        for (uint nread = 0; nread < st.st_size; )
            nread += read(0, exec + nread, sizeof(exec) - nread);

        write(1, exec, st.st_size);
        write(1, exec + st.st_size, sizeof(exec) - st.st_size);
    }
    memset(exec, 0, sizeof(exec));
    for (uint i = 0; i < EGOS_BIN_MAX_NUM - EGOS_BIN_NUM; i++) {
        write(1, exec, sizeof(exec));
    }

    /* Write the file system into disk.img */
    write(1, fs, sizeof(fs));
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image (tools/disk.img)\n");
    return 0;
}

int getsize() { return FILE_SYS_DISK_SIZE / BLOCK_SIZE; }

int setsize() { assert(0); }

int ramread(inode_intf bs, uint ino, block_no offset, block_t* block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(inode_intf bs, uint ino, block_no offset, block_t* block) {
    memcpy(fs + offset * BLOCK_SIZE, block, BLOCK_SIZE);
    return 0;
}

inode_intf ramdisk_init() {
    inode_store_t* ramdisk = malloc(sizeof(*ramdisk));

    ramdisk->read = (void*)ramread;
    ramdisk->write = (void*)ramwrite;
    ramdisk->getsize = (void*)getsize;
    ramdisk->setsize = (void*)setsize;

    return ramdisk;
}

