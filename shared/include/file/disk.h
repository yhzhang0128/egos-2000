#pragma once

#define BLOCK_SIZE            512
#define PAGING_DEV_SIZE       1024 * 1024
#define GRASS_EXEC_SIZE       1024 * 1024
#define GRASS_EXEC_SEGMENT    (GRASS_EXEC_SIZE / 8 / BLOCK_SIZE)

#define GRASS_EXEC_START      PAGING_DEV_SIZE / BLOCK_SIZE
#define SYS_PROC_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT
#define SYS_FILE_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 2
#define SYS_DIR_EXEC_START    GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 3
#define SYS_SHELL_EXEC_START  GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 4

struct block_store {
    int (*read)(int block_no, char* dst);
    int (*write)(int block_no, char* src);
};
