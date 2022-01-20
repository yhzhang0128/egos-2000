#pragma once

#define BLOCK_SIZE          512
#define PAGING_DEV_SIZE     1024 * 1024
#define GRASS_EXEC_SIZE     1024 * 1024
#define GRASS_EXEC_SEGMENT  (GRASS_EXEC_SIZE / 8 / BLOCK_SIZE)

#define GRASS_EXEC_START    PAGING_DEV_SIZE / BLOCK_SIZE
#define FS_EXEC_START       GRASS_EXEC_START + GRASS_EXEC_SEGMENT

struct block_store {
    int (*read)(int block_no, int nblocks, char* dst);
    int (*write)(int block_no, int nblocks, char* src);
};
