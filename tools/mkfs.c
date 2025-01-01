/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: create the disk image (disk.img) and ROM image (bootROM.bin)
 * The disk image should be exactly 4MB:
 *     2MB holds the executables of EGOS and system servers;
 *     2MB is managed by a file system.
 * This image file should be programmed to the microSD card.
 *
 * The ROM image should be exactly 8MB:
 *     4MB holds the VexRiscv processor FPGA binary;
 *     4MB holds the disk image described above.
 * This image file should be programmed to the ROM chip on the Arty board.
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

#define EGOS_BIN_NUM 5
char* egos_binaries[] = {"./qemu/egos.bin", "../build/release/sys_proc.elf",
                         "../build/release/sys_terminal.elf",
                         "../build/release/sys_file.elf",
                         "../build/release/sys_shell.elf"};

#define BIN_DIR_INODE 6
char bin_dir[256] = "./   6 ../   0 ";
char* contents[]  = {
    "./   0 ../   0 home/   1 bin/   6 ",
    "./   1 ../   0 yunhao/   2 rvr/   3 yacqub/   4 ",
    "./   2 ../   1 README   5 ",
    "./   3 ../   1 ",
    "./   4 ../   1 ",
    "With only 2000 lines of code, egos-2000 implements boot loader, microSD "
     "driver, tty driver, memory translation, interrupt handling, preemptive "
     "scheduler, system call, file system, shell, a UDP/Ethernet demo, several "
     "user commands, and the `mkfs/mkrom` tools.",
    bin_dir};

#define SIZE_2MB 2 * 1024 * 1024

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

int main() {
    assert(EGOS_BIN_DISK_SIZE == SIZE_2MB && FILE_SYS_DISK_SIZE == SIZE_2MB);

    /* Write the EGOS binaries into exec[] */
    printf("[INFO] Load %d kernel binary files\n", EGOS_BIN_NUM);
    for (uint i = 0; i < EGOS_BIN_NUM; i++) {
        int file_size = load_file(egos_binaries[i],
                                  exec + i * EGOS_BIN_MAX_NBLOCK * BLOCK_SIZE);
        printf("[INFO] Load %s: %d bytes\n", egos_binaries[i], file_size);
    }

    /* Initialize the file system using fs[] as ramdisk */
    printf("MKFS is using *%s*\n", FILESYS == 0 ? "mydisk" : "treedisk");
    inode_intf ramdisk_init();
    inode_intf ramdisk = ramdisk_init();
    (FILESYS == 0) ? assert(mydisk_create(ramdisk, 0, NINODES) >= 0)
                   : assert(treedisk_create(ramdisk, 0, NINODES) >= 0);
    inode_intf filesys =
        (FILESYS == 0) ? mydisk_init(ramdisk, 0) : treedisk_init(ramdisk, 0);

    /* Write to inode 0..BIN_DIR_INODE-1 */
    for (uint ino = 0; ino < BIN_DIR_INODE; ino++) {
        printf("[INFO] Load ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
        strncpy(inode, contents[ino], BLOCK_SIZE);
        filesys->write(filesys, ino, 0, (void*)inode);
    }

    /* Write to one inode for every application under /bin */
    uint app_ino = BIN_DIR_INODE + 1;
    DIR* dp      = opendir("../build/release/user");
    assert(dp != NULL);
    for (struct dirent* ep = readdir(dp); ep != NULL; ep = readdir(dp))
        if (strstr(ep->d_name, ".elf")) {
            sprintf(tmp, "../build/release/user/%s", ep->d_name);
            int file_size = load_file(tmp, inode);
            printf("[INFO] Load ino=%d, %s: %d bytes\n", app_ino, ep->d_name,
                   file_size);

            /* Write the ELF format application binary into inode app_ino */
            for (uint b = 0; b * BLOCK_SIZE < file_size; b++)
                filesys->write(filesys, app_ino, b,
                               (void*)(inode + b * BLOCK_SIZE));

            /* Add a file entry into directory /bin */
            ep->d_name[strlen(ep->d_name) - 4] = 0;
            sprintf(tmp, "%s%4d ", ep->d_name, app_ino++);
            strcat(bin_dir, tmp);
        }
    closedir(dp);
    filesys->write(filesys, BIN_DIR_INODE, 0, (void*)bin_dir);
    printf("[INFO] Load ino=%d, %s\n", BIN_DIR_INODE, bin_dir);

    /* Create the disk image file */
    int fd    = open("disk.img", O_CREAT | O_WRONLY, 0666);
    int size1 = write(fd, exec, SIZE_2MB);
    int size2 = write(fd, fs, SIZE_2MB);
    close(fd);
    assert(size1 + size2 == SIZE_2MB * 2);
    printf("[INFO] Finish making the disk image (tools/disk.img)\n");

    /* Create the ROM image file */
    fd = open("bootROM.bin", O_CREAT | O_WRONLY, 0666);
    assert(load_file(CPU_BIN_FILE, vexriscv) < SIZE_2MB * 2);
    int size0 = write(fd, vexriscv, SIZE_2MB * 2);
    size1     = write(fd, exec, SIZE_2MB);
    size2     = write(fd, fs, SIZE_2MB);
    close(fd);
    assert(size0 + size1 + size2 == SIZE_2MB * 4);
    printf("[INFO] Finish making the bootROM binary (tools/bootROM.bin)\n");
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

    ramdisk->read    = (void*)ramread;
    ramdisk->write   = (void*)ramwrite;
    ramdisk->getsize = (void*)getsize;
    ramdisk->setsize = (void*)setsize;

    return ramdisk;
}
