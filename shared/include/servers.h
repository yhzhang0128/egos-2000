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
    enum {
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

#define DIR_ROOT_INO    1
#define DIR_NAME_SIZE   32
struct dir_request {
    enum {
          DIR_UNUSED,
          DIR_LOOKUP,
          DIR_INSERT,
          DIR_REMOVE
    } type;
    int ino;
    char name[DIR_NAME_SIZE];
};

struct dir_reply {
    enum dir_status { DIR_OK, DIR_ERROR } status;
    int ino;
};

#define CMD_NARGS       16
#define CMD_ARG_LEN     32
struct proc_request {
    enum {
          PROC_SPAWN,
          PROC_KILLED
    } type;
    int pid;
    int argc;
    char argv[CMD_NARGS][CMD_ARG_LEN];
};

struct proc_reply {
    enum {
          CMD_OK,
          CMD_ERROR
    } type;
};
