/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: generate disk image (disk.img) and ROM image (fpgaROM.bin)
 * The disk image should be exactly 4MB:
 *     2MB holds the executables of EGOS and system servers;
 *     2MB is managed by a file system.
 * This disk image should be programmed to the microSD card.
 *
 * The ROM image should be exactly 8MB:
 *     4MB holds the VexRiscv processor FPGA binary;
 *     4MB holds the disk image described above.
 * This ROM image should be programmed to the ROM chip on the FPGA board.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "inode.h"

char* egos_binaries[] = {"./egos.bin",
                         "../build/release/sys_proc.elf",
                         "../build/release/sys_terminal.elf",
                         "../build/release/sys_file.elf",
                         "../build/release/sys_shell.elf",
                         "./images/Bohr.bmp" /* for the video demo app */};
#define EGOS_BIN_NUM ((sizeof(egos_binaries) / sizeof(char*)))

char bin_dir[256] = "./   6 ../   0 ";
char* contents[]  = {
    "./   0 ../   0 home/   1 bin/   6 ",
    "./   1 ../   0 yunhao/   2 rvr/   3 yacqub/   4 ",
    "./   2 ../   1 README   5 ",
    "./   3 ../   1 ",
    "./   4 ../   1 ",
    "With only 2000 lines of code, egos-2000 implements boot loader, SD card "
     "driver, tty driver, virtual memory with page tables, interrupt and "
     "exception handling, preemptive scheduler, system call, file system, "
     "shell, an Ethernet/UDP demo, several user commands, and the mkfs tool. "
     "Moreover, the EGOS book (https://egos.fun) contains 9 course projects.",
    bin_dir};
#define BIN_DIR_INODE ((sizeof(contents) / sizeof(char*)) - 1)

char inode[SIZE_2MB], tmp[512];
char vexriscv[SIZE_2MB * 2], exec[SIZE_2MB], fs[SIZE_2MB];

int load_file(char* file_name, char* dst) {
    struct stat st;
    stat(file_name, &st);
    int fd = open(file_name, O_RDONLY);
    for (uint nread = 0; nread < st.st_size;)
        nread += read(fd, dst + nread, st.st_size - nread);
    close(fd);

    return st.st_size;
}

int getsize(inode_intf bs, uint ino) { return FILE_SYS_DISK_SIZE / BLOCK_SIZE; }

int setsize(inode_intf bs, uint ino, uint newsize) { assert(0); }

int ramread(inode_intf bs, uint ino, uint offset, block_t* block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(inode_intf bs, uint ino, uint offset, block_t* block) {
    memcpy(fs + offset * BLOCK_SIZE, block, BLOCK_SIZE);
    return 0;
}

int main() {
    /* Write the kernel and system server binaries into exec[]. */
    printf("[INFO] Load %ld kernel binary files\n", EGOS_BIN_NUM);
    for (uint i = 0; i < EGOS_BIN_NUM; i++) {
        int sz = load_file(egos_binaries[i], exec + i * EGOS_BIN_MAX_NBYTE);
        printf("[INFO] Load %s: %d bytes\n", egos_binaries[i], sz);
    }

    /* Initialize the file system using the fs[] buffer as a ramdisk. */
    printf("MKFS is using *%s*\n", FILESYS == 0 ? "mydisk" : "treedisk");
    struct inode_store ramdisk = (struct inode_store){.read    = ramread,
                                                      .write   = ramwrite,
                                                      .getsize = getsize,
                                                      .setsize = setsize};
    (FILESYS == 0) ? assert(mydisk_create(&ramdisk, 0, NINODES) >= 0)
                   : assert(treedisk_create(&ramdisk, 0, NINODES) >= 0);
    inode_intf filesys =
        (FILESYS == 0) ? mydisk_init(&ramdisk, 0) : treedisk_init(&ramdisk, 0);

    /* Write to inode 0..BIN_DIR_INODE-1 in the file system. */
    for (uint ino = 0; ino < BIN_DIR_INODE; ino++) {
        printf("[INFO] Load ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
        strncpy(inode, contents[ino], BLOCK_SIZE);
        filesys->write(filesys, ino, 0, (void*)inode);
    }

    /* Write to one inode for each user application. */
    uint app_ino = BIN_DIR_INODE + 1;
    DIR* dp      = opendir("../build/release/user");
    assert(dp != NULL);
    for (struct dirent* ep = readdir(dp); ep != NULL; ep = readdir(dp))
        if (strstr(ep->d_name, ".elf")) {
            sprintf(tmp, "../build/release/user/%s", ep->d_name);
            int file_size = load_file(tmp, inode);
            printf("[INFO] Load ino=%d, %s: %d bytes\n", app_ino, ep->d_name,
                   file_size);

            /* Write the ELF format application binary into inode app_ino. */
            for (uint b = 0; b * BLOCK_SIZE < file_size; b++)
                filesys->write(filesys, app_ino, b,
                               (void*)(inode + b * BLOCK_SIZE));

            /* Add the corresponding file entry into the /bin directory. */
            ep->d_name[strlen(ep->d_name) - 4] = 0;
            sprintf(tmp, "%s%4d ", ep->d_name, app_ino++);
            strcat(bin_dir, tmp);
        }
    closedir(dp);
    filesys->write(filesys, BIN_DIR_INODE, 0, (void*)bin_dir);
    printf("[INFO] Load ino=%ld, %s\n", BIN_DIR_INODE, bin_dir);

    /* Generate the disk image file. */
    int fd  = open("disk.img", O_CREAT | O_WRONLY, 0666);
    int sz1 = write(fd, exec, SIZE_2MB);
    sz1 += write(fd, fs, SIZE_2MB);
    close(fd);

    /* Generate the ROM image files. */
    fd = open("fpgaROM.bin", O_CREAT | O_WRONLY, 0666);
    assert(load_file(CPU_BIN_FILE, vexriscv) < SIZE_2MB * 2);
    int sz2 = write(fd, vexriscv, SIZE_2MB * 2);
    sz2 += write(fd, exec, SIZE_2MB);
    sz2 += write(fd, fs, SIZE_2MB);
    close(fd);

    fd      = open("qemuROM.bin", O_CREAT | O_WRONLY, 0666);
    int sz3 = write(fd, exec, SIZE_2MB);
    for (int i = 0; i < 15; i++) sz3 += write(fd, fs, SIZE_2MB);
    close(fd);

    assert(sz1 == SIZE_2MB * 2 && sz2 == SIZE_2MB * 4 && sz3 == SIZE_2MB * 16);
    printf("[INFO] Finish making the image files\n");
    return 0;
}
