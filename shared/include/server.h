#pragma once

#include "fs.h"

enum grass_servers {
    GPID_UNUSED,
    GPID_PROCESS,
    GPID_FILE,
    GPID_DIR,
    GPID_SHELL,
    GPID_USER_START
};

struct file_request {
    enum file_op {
                  FILE_UNUSED,
                  FILE_READ,
                  FILE_WRITE,
    } type;
    unsigned int ino;      // inode number
    unsigned int offset;   // offset
    block_t block;
};

struct file_reply {
    enum file_status { FILE_OK, FILE_ERROR } status;
    block_t block;
};

