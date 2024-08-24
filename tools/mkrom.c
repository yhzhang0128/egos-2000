/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: create the ROM image file (bootROM.bin)
 * The ROM image should be 8MB:
 *     4MB holds the VexRiscv processor FPGA binary;
 *     4MB holds the disk image produced by mkfs.
 * This image file should be programmed to the ROM chip on the Arty board.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIZE_4MB  4 * 1024 * 1024

uint vexriscv_size, disk_size;
char mem_vexriscv[SIZE_4MB], mem_disk [SIZE_4MB];

int load_file(char* file_name, char* print_name, char* dst) {
    struct stat st;
    stat(file_name, &st);
    printf("[INFO] %s has 0x%.6x bytes\n", print_name, (int)st.st_size);

    freopen(file_name, "r", stdin);
    for (uint nread = 0; nread < st.st_size; )
        nread += read(0, dst + nread, st.st_size - nread);
    fclose(stdin);

    return st.st_size;
}

int main(int argc, char** argv) {
    vexriscv_size = load_file(CPU_BIN_FILE, "VexRiscv binary", mem_vexriscv);
    disk_size     = load_file("disk.img",   "Disk     image ", mem_disk    );

    assert(vexriscv_size <= SIZE_4MB && disk_size  == SIZE_4MB);

    freopen("bootROM.bin", "w", stdout);
    for (uint i = 0; i < SIZE_4MB; i++) putchar(mem_vexriscv[i]);
    for (uint i = 0; i < SIZE_4MB; i++) putchar(mem_disk[i]);
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the bootROM binary (tools/bootROM.bin)\n");
    return 0;
}
