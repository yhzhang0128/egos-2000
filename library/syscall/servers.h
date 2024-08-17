#pragma once

#include "inode.h"
#define SYSCALL_MSG_LEN    1024

void exit(int status);
int term_read(char* buf, uint len);
void term_write(char* str, uint len);
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
#define TERM_BUF_SIZE   512
struct term_request {
    enum { TERM_INPUT, TERM_OUTPUT } type;
    uint len;
    char buf[TERM_BUF_SIZE];
};

struct term_reply {
    uint len;
    char buf[TERM_BUF_SIZE];
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
