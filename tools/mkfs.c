/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: create the disk image file (disk.img)
 * The disk image should be exactly 4MB:
 *     2MB contains executables of EGOS;
 *     2MB is managed by a file system.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "disk.h"
#include "file.h"

#define EGOS_BIN_NUM 4
char* egos_binaries[] = {"./qemu/egos.bin",
                         "../build/release/sys_proc.elf",
                         "../build/release/sys_file.elf",
                         "../build/release/sys_shell.elf"};

/* Inode - File/Directory mappings:
#0: /              #1: /home                #2: /home/yunhao  #3: /home/rvr
#4: /home/yacqub   #5: /home/yunhao/README  #6: /bin          #7: /bin/cat
#8: /bin/cd        #9: /bin/clock           #10:/bin/crash1   #11:/bin/crash2
#12:/bin/echo      #13:/bin/ls              #14:/bin/mt       #15:/bin/pwd
#16:/bin/udp_hello
*/
#define NINODE 17
char* contents[] = {
                    "./   0 ../   0 home/   1 bin/   6 ",
                    "./   1 ../   0 yunhao/   2 rvr/   3 yacqub/   4 ",
                    "./   2 ../   1 README   5 ",
                    "./   3 ../   1 ",
                    "./   4 ../   1 ",
                    "With only 2000 lines of code, egos-2000 implements boot loader, microSD driver, tty driver, memory translation, interrupt handling, preemptive scheduler, system call, file system, shell, a UDP/Ethernet demo, several user commands, and the `mkfs/mkrom` tools.",
                    "./   6 ../   0 cat   7 cd   8 clock    9 crash1  10 crash2  11 echo  12 ls  13 mt  14 pwd  15 udp_hello  16",
                    "#../build/release/cat.elf",
                    "#../build/release/cd.elf",
                    "#../build/release/clock.elf",
                    "#../build/release/crash1.elf",
                    "#../build/release/crash2.elf",
                    "#../build/release/echo.elf",
                    "#../build/release/ls.elf",
                    "#../build/release/mt.elf",
                    "#../build/release/pwd.elf",
                    "#../build/release/udp_hello.elf"};

char exec[EGOS_BIN_MAX_NBLOCK * BLOCK_SIZE], fs[FILE_SYS_DISK_SIZE];

inode_intf ramdisk_init();

int main() {
    /* Make the file system into char fs[] */
    inode_intf ramdisk = ramdisk_init();
    assert(treedisk_create(ramdisk, 0, NINODES) >= 0);
    inode_intf treedisk = treedisk_init(ramdisk, 0);

    char buf[EGOS_BIN_MAX_NBLOCK * BLOCK_SIZE];
    for (uint ino = 0; ino < NINODE; ino++) {
        if (contents[ino][0] != '#') {
            fprintf(stderr, "[INFO] Loading ino=%d, %ld bytes\n", ino, strlen(contents[ino]));
            strncpy(buf, contents[ino], BLOCK_SIZE);
            treedisk->write(treedisk, ino, 0, (void*)buf);
        } else {
            struct stat st;
            char* file_name = &contents[ino][1];
            stat(file_name, &st);

            freopen(file_name, "r", stdin);
            for (uint nread = 0; nread < st.st_size; )
                nread += read(0, buf + nread, st.st_size - nread);

            fprintf(stderr, "[INFO] Loading ino=%d, %s: %d bytes\n", ino, file_name, (int)st.st_size);
            for (uint b = 0; b * BLOCK_SIZE < st.st_size; b++)
                treedisk->write(treedisk, ino, b, (void*)(buf + b * BLOCK_SIZE));
        }
    }

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
    for (uint i = 0; i < EGOS_BIN_MAX_NUM - EGOS_BIN_NUM; i++)
        write(1, exec, sizeof(exec));

    /* Write the file system into disk.img */
    write(1, fs, sizeof(fs));
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the disk image (tools/disk.img)\n");
    return 0;
}

int getsize() { return FILE_SYS_DISK_SIZE / BLOCK_SIZE; }

int setsize() { assert(0); }

int ramread(inode_intf bs, uint ino, block_no offset, block_t *block) {
    memcpy(block, fs + offset * BLOCK_SIZE, BLOCK_SIZE);
    return 0;
}

int ramwrite(inode_intf bs, uint ino, block_no offset, block_t *block) {
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

