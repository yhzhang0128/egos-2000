/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block and 
 * spawns some kernel processes, including file system and shell
 */

#include "egos.h"
#include "grass.h"

struct earth *earth = (void*)EARTH_ADDR;

static int elf_fs_read(int block_no, int nblocks, char* dst) {
    return earth->disk_read(FS_EXEC_START + block_no, nblocks, dst);    
}

int main() {
    SUCCESS("Enter the grass layer");
    char* buf = malloc(512);
    free(buf);
    INFO("stack variable @0x%.8x and heap variable @0x%.8x", &buf, buf);

    struct block_store bs;

    INFO("Load the file system kernel process");
    bs.read = elf_fs_read;
    /* elf_load(&bs, earth); */

    return 0;
}
