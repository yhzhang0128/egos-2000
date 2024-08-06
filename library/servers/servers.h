#pragma once

#include "inode.h"
#define SYSCALL_MSG_LEN    1024

void exit(int status);
int dir_lookup(int dir_ino, char* name);
int file_read(int file_ino, uint offset, char* block);

enum grass_servers {
    GPID_ALL = -1,
    GPID_UNUSED,
    GPID_PROCESS,
    GPID_TERMINAL,
    GPID_FILE,
    GPID_SHELL,
    GPID_USER_START
};

/* GPID_PROCESS */
#define CMD_NARGS       16
#define CMD_ARG_LEN     32
struct proc_request {
    enum {
          PROC_SPAWN,
          PROC_EXIT,
          PROC_KILLALL
    } type;
    int argc;
    char argv[CMD_NARGS][CMD_ARG_LEN];
};

struct proc_reply {
    enum {
          CMD_OK,
          CMD_ERROR
    } type;
};

/* GPID_TERMINAL */
#define BUFSIZE     256
struct term_request {
    enum {
        TERM_INPUT,
        TERM_OUTPUT
    } type;
    int len;
    char buf[BUFSIZE];
};

struct term_reply {
    enum term_status { TERM_OK, TERM_ERROR } status;
    char buf[BUFSIZE];
};

/* GPID_FILE */
struct file_request {
    enum {
          FILE_UNUSED,
          FILE_READ,
          FILE_WRITE,
    } type;
    uint ino;
    uint offset;
    block_t block;
};

struct file_reply {
    enum file_status { FILE_OK, FILE_ERROR } status;
    block_t block;
};
