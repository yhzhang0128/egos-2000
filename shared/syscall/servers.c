#include <string.h>

#include "egos.h"
#include "print.h"
#include "syscall.h"
#include "servers.h"

int dir_lookup(int dir_ino, char* name) {
    int sender;
    struct dir_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = DIR_LOOKUP;
    req.ino = dir_ino;
    strcpy(req.name, name);
    sys_send(GPID_DIR, (void*)&req, sizeof(struct dir_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_DIR)
        FATAL("dir_lookup expects message from GPID_DIR");

    struct dir_reply *reply = (void*)buf;
    return reply->status == DIR_OK? reply->ino : -1;
}

int file_read(int file_ino, int offset, char* block) {
    int sender;
    struct file_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = FILE_READ;
    req.ino = file_ino;
    req.offset = offset;
    sys_send(GPID_FILE, (void*)&req, sizeof(struct file_request));

    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_FILE)
        FATAL("file_read expects message from GPID_FILE");

    struct file_reply *reply = (void*)buf;
    memcpy(block, reply->block.bytes, BLOCK_SIZE);
}
