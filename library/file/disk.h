#pragma once

#define BLOCK_SIZE            512

#define EGOS_BIN_MAX_NUM      8
#define EGOS_BIN_DISK_SIZE    1024 * 1024 * 2
#define FILE_SYS_DISK_SIZE    1024 * 1024 * 2
#define FILE_SYS_DISK_START   (EGOS_BIN_DISK_SIZE / BLOCK_SIZE)

#define EGOS_BIN_MAX_NBLOCK   (EGOS_BIN_DISK_SIZE / EGOS_BIN_MAX_NUM / BLOCK_SIZE)
#define SYS_PROC_EXEC_START   EGOS_BIN_MAX_NBLOCK * 1
#define SYS_TERM_EXEC_START   EGOS_BIN_MAX_NBLOCK * 2
#define SYS_FILE_EXEC_START   EGOS_BIN_MAX_NBLOCK * 3
#define SYS_SHELL_EXEC_START  EGOS_BIN_MAX_NBLOCK * 4

typedef unsigned int block_no;      /* index of a block */

typedef struct block {
    char bytes[BLOCK_SIZE];
} block_t;
