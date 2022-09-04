#pragma once

#define BLOCK_SIZE            512
#define PAGING_DEV_SIZE       1024 * 1024
#define GRASS_NEXEC           8
#define GRASS_EXEC_SIZE       1024 * 1024
#define FS_DISK_SIZE          1024 * 1024 * 2
#define GRASS_EXEC_SEGMENT    (GRASS_EXEC_SIZE / GRASS_NEXEC / BLOCK_SIZE)

#define GRASS_EXEC_START      PAGING_DEV_SIZE / BLOCK_SIZE
#define SYS_PROC_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT
#define SYS_FILE_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 2
#define SYS_DIR_EXEC_START    GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 3
#define SYS_SHELL_EXEC_START  GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 4

#define GRASS_FS_START        (PAGING_DEV_SIZE + GRASS_EXEC_SIZE) / BLOCK_SIZE

typedef unsigned int block_no;      /* index of a block */

typedef struct block {
    char bytes[BLOCK_SIZE];
} block_t;